all:
	g++ -o serverC central.cc -std=c++11
	g++ -o serverT serverT.cc -std=c++11
	g++ -o serverS serverS.cc -std=c++11
	g++ -o serverP serverP.cc -std=c++11
	g++ -o clientA clientA.cc -std=c++11
	g++ -o clientB clientB.cc -std=c++11

serverC:
	./serverC

serverT:
	./serverT

serverS:
	./serverS

serverP:
	./serverP


.PHONY: all serverC serverT serverS serverP
