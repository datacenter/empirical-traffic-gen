all: 
	cd src; $(MAKE); cd ..;
	rm -rf bins
	mkdir bins
	cp src/client bins/.
	cp src/server bins/.

clean: 
	cd src; $(MAKE) clean; cd ..;
	rm -rf bins

