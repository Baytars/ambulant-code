all: ambulantmodule.so

ambulantmodule.cpp: ambulantscan.py ambulantsupport.py ambulantgen.py
	python ambulantscan.py

ambulantmodule.so: ambulantmodule.cpp setup.py
	python setup.py build
