#include <assert.h>

#include <node.h>
#include <node_buffer.h>
#include <v8.h>

#include "bsdiff.h"

using namespace node;
using namespace v8;

namespace node_bsdiff {
namespace {

class async_stub : public bsdiff_dat {
 public:
  int err;

  Persistent<Value> cur_handle;
  Persistent<Value> ref_handle;
  Persistent<Value> ctrl_handle;
  Persistent<Value> diff_handle;
  Persistent<Value> xtra_handle;

  Persistent<Function> callback;

  async_stub() : err(0) {
  }

  ~async_stub() {
    cur_handle.Dispose();
    ref_handle.Dispose();
    ctrl_handle.Dispose();
    diff_handle.Dispose();
    xtra_handle.Dispose();
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

  Local<Array> ctrl = Array::New(shim->ctrl.size());

  int i = 0;
  std::vector<int>::iterator it;
  for (it = shim->ctrl.begin(); it < shim->ctrl.end(); ++it, ++i)
    ctrl->Set(i, Integer::New(*it));

  Buffer *diff = Buffer::New(shim->diff, shim->difflen, DeleteMemory, NULL);
  Buffer *xtra = Buffer::New(shim->xtra, shim->xtralen, DeleteMemory, NULL);

  Handle<Value> argv[] = { Null(), ctrl, diff->handle_, xtra->handle_ };
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

  shim->cur_handle = Persistent<Value>::New(cur);
  shim->ref_handle = Persistent<Value>::New(ref);

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
      !args[2]->IsArray() ||            // control
      !Buffer::HasInstance(args[3]) ||  // diff
      !Buffer::HasInstance(args[4]) ||  // extra
      !args[5]->IsFunction())           // callback
    return ThrowTypeError("Invalid arguments");

  uint32_t curlen = args[0]->Uint32Value();
  Local<Object> ref = args[1]->ToObject();
  Local<Array> ctrl = Local<Array>::Cast(args[2]->ToObject());
  Local<Object> diff = args[3]->ToObject();
  Local<Object> xtra = args[4]->ToObject();
  Local<Function> callback = Local<Function>::Cast(args[5]);

  uv_work_t *req = new uv_work_t;
  async_stub *shim = new async_stub;

  for (uint32_t i = 0; i < ctrl->Length(); ++i)
    shim->ctrl.push_back(ctrl->Get(i)->Uint32Value());

  shim->refdat = Buffer::Data(ref);
  shim->diff = Buffer::Data(diff);
  shim->xtra = Buffer::Data(xtra);

  shim->curlen = curlen;
  shim->reflen = Buffer::Length(ref);
  shim->difflen = Buffer::Length(diff);
  shim->xtralen = Buffer::Length(xtra);

  shim->ref_handle = Persistent<Value>::New(ref);
  shim->ctrl_handle = Persistent<Value>::New(ctrl);
  shim->diff_handle = Persistent<Value>::New(diff);
  shim->xtra_handle = Persistent<Value>::New(xtra);

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
