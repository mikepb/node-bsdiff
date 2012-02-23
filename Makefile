REPORTER=dot

# prefer installed scripts
PATH:=./node_modules/.bin:${PATH}

build:
	if [ ! -d build ]; then node-waf configure; fi
	node-waf build

coffee:
	coffee --bare --compile --output lib src/coffee

clean:
	rm -rf build

distclean: clean
	rm -rf lib node_modules

preinstall: build clean
	if [ ! -f ./lib/index.js ]; then make coffee; fi
	rm -rf Makefile src test wscript

prepublish: coffee

test: build coffee
	mocha --reporter $(REPORTER) --timeout 30000 test/*-test.coffee

.PHONY: build coffee clean distclean preinstall prepublish test
