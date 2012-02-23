/*

    Copyright 2012 Michael Phan-Ba

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

*/

#include <vector>

#include <node.h>
#include <node_buffer.h>
#include <v8.h>

#include "boost/endian.hpp"
#ifdef BOOST_BIG_ENDIAN
# include "gnuclib/byteswap.h"
#endif

#include "bsdiff.h"

using namespace node;
using namespace v8;

namespace node_bsdiff {
namespace {

class async_stub : public bsdiff_dat {
 public:
  int err;

  Persistent<Value> curHandle;
  Persistent<Value> refHandle;
  Persistent<Value> ctrlHandle;
  Persistent<Value> diffHandle;
  Persistent<Value> xtraHandle;

  Persistent<Function> callback;

  async_stub() : bsdiff_dat(), err(0) {
  }

  ~async_stub() {
    curHandle.Dispose();
    refHandle.Dispose();
    ctrlHandle.Dispose();
    diffHandle.Dispose();
    xtraHandle.Dispose();
    callback.Dispose();
  }

};

static inline Handle<Value> ThrowTypeError(const char *err) {
  return ThrowException(Exception::TypeError(String::New(err)));
}

static inline void Error(async_stub *shim) {
  const char *msg = shim->err == -1 ? "Corrupt data" : "Internal error";
  Handle<Value> argv[] = { Exception::Error(String::New(msg)) };
  TryCatch tryCatch;
  shim->callback->Call(Context::GetCurrent()->Global(), 1, argv);
  if (tryCatch.HasCaught()) FatalException(tryCatch);
}

static void DeleteMemory(char *data, void *hint) {
  delete data;
}

static void AsyncDiff(uv_work_t *req) {
  async_stub *shim = static_cast<async_stub *>(req->data);
  shim->err = bsdiff(shim);
}

static void AfterDiff(uv_work_t *req) {
  HandleScope scope;
  async_stub *shim = static_cast<async_stub *>(req->data);

  if (shim->err != 0) return Error(shim);

#ifdef BOOST_BIG_ENDIAN
  for (size_t i = shim->ctrl.size() - 1; i >= 0; --it)
    shim->ctrl[i] = bswap_32(shim->ctrl[i]);
#endif

  Buffer *ctrl = Buffer::New(reinterpret_cast<char *>(shim->ctrl.data()),
                             shim->ctrl.size() * sizeof(int));
  Buffer *diff = Buffer::New(shim->diff, shim->difflen, DeleteMemory, NULL);
  Buffer *xtra = Buffer::New(shim->xtra, shim->xtralen, DeleteMemory, NULL);

  Handle<Value> argv[] = { Null(), ctrl->handle_, diff->handle_, xtra->handle_ };
  TryCatch tryCatch;
  shim->callback->Call(Context::GetCurrent()->Global(), 4, argv);
  if (tryCatch.HasCaught()) FatalException(tryCatch);

  delete shim;
  delete req;
}

static void AsyncPatch(uv_work_t *req) {
  async_stub *shim = static_cast<async_stub *>(req->data);
  shim->err = bspatch(shim);
}

static void AfterPatch(uv_work_t *req) {
  HandleScope scope;
  async_stub *shim = static_cast<async_stub *>(req->data);

  if (shim->err != 0) return Error(shim);

  Buffer *cur = Buffer::New(shim->curdat, shim->curlen, DeleteMemory, NULL);

  Handle<Value> argv[] = { Null(), cur->handle_ };
  TryCatch tryCatch;
  shim->callback->Call(Context::GetCurrent()->Global(), 2, argv);
  if (tryCatch.HasCaught()) FatalException(tryCatch);

  delete shim;
  delete req;
}

} // anonymous namespace

Handle<Value> Diff(const Arguments& args) {
  HandleScope scope;

  if (args.Length() != 3 ||
      !Buffer::HasInstance(args[0]) ||  // current
      !Buffer::HasInstance(args[1]) ||  // reference
      !args[2]->IsFunction())           // callback
    return ThrowTypeError("Invalid arguments");

  Local<Object> cur = args[0]->ToObject();
  Local<Object> ref = args[1]->ToObject();
  Local<Function> callback = Local<Function>::Cast(args[2]);

  uv_work_t *req = new uv_work_t;
  async_stub *shim = new async_stub;

  shim->curdat = Buffer::Data(cur);
  shim->refdat = Buffer::Data(ref);

  shim->curlen = Buffer::Length(cur);
  shim->reflen = Buffer::Length(ref);

  shim->curHandle = Persistent<Value>::New(cur);
  shim->refHandle = Persistent<Value>::New(ref);

  shim->callback = Persistent<Function>::New(callback);

  req->data = shim;
  uv_queue_work(uv_default_loop(), req, AsyncDiff, AfterDiff);

  return Undefined();
}

Handle<Value> Patch(const Arguments& args) {
  HandleScope scope;

  if (args.Length() != 6 ||
      !args[0]->IsNumber() ||           // current
      !Buffer::HasInstance(args[1]) ||  // reference
      !Buffer::HasInstance(args[2]) ||  // control
      !Buffer::HasInstance(args[3]) ||  // diff
      !Buffer::HasInstance(args[4]) ||  // extra
      !args[5]->IsFunction())           // callback
    return ThrowTypeError("Invalid arguments");

  uint32_t curlen = args[0]->Uint32Value();
  Local<Object> ref = args[1]->ToObject();
  Local<Object> ctrl = args[2]->ToObject();
  Local<Object> diff = args[3]->ToObject();
  Local<Object> xtra = args[4]->ToObject();
  Local<Function> callback = Local<Function>::Cast(args[5]);

  uv_work_t *req = new uv_work_t;
  async_stub *shim = new async_stub;

  const int *ctrldat = reinterpret_cast<int *>(Buffer::Data(ctrl));
  const size_t ctrllen = Buffer::Length(ctrl);

#ifdef BOOST_BIG_ENDIAN
  for (size_t i = ctrllen - 1; i >= 0; --it)
    ctrldat[i] = bswap_32(ctrldat[i]);
#endif

  shim->ctrl.assign(ctrldat, ctrldat + ctrllen);

  shim->refdat = Buffer::Data(ref);
  shim->diff = Buffer::Data(diff);
  shim->xtra = Buffer::Data(xtra);

  shim->curlen = curlen;
  shim->reflen = Buffer::Length(ref);
  shim->difflen = Buffer::Length(diff);
  shim->xtralen = Buffer::Length(xtra);

  shim->refHandle = Persistent<Value>::New(ref);
  shim->ctrlHandle = Persistent<Value>::New(ctrl);
  shim->diffHandle = Persistent<Value>::New(diff);
  shim->xtraHandle = Persistent<Value>::New(xtra);

  shim->callback = Persistent<Function>::New(callback);

  req->data = shim;
  uv_queue_work(uv_default_loop(), req, AsyncPatch, AfterPatch);

  return Undefined();
}

extern "C" {

static void init(Handle<Object> target) {
  HandleScope scope;
  NODE_SET_METHOD(target, "diff", Diff);
  NODE_SET_METHOD(target, "patch", Patch);
}

NODE_MODULE(bsdiff, init);

} // extern
} // namespace node_bsdiff
