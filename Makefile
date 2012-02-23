REPORTER=dot

# prefer installed scripts
PATH:=./node_modules/.bin:${PATH}

build:
	if [ ! -d build ]; then node-waf configure; fi
	node-waf build

coffee: lib/index.js
	coffee --bare --compile --output lib src/coffee

clean:
	rm -rf build

distclean: clean
	rm -rf lib node_modules

test: build coffee
	mocha --reporter $(REPORTER) test/*-test.coffee

.PHONY: build coffee clean distclean test
