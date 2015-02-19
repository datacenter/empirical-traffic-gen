all: 
	cd src; $(MAKE); cd ..;
	rm -rf bin
	mkdir bin
	cp src/client bin/.
	cp src/server bin/.

clean: 
	cd src; $(MAKE) clean; cd ..;
	rm -rf bin

