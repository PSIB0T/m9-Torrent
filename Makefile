PEER_FILES = misc peer peer_global fileFunc
TRACKER_FILES = misc tracker tracker_global

all: mainT mainP

mainP: mainP.o
	g++ $(foreach var,$(PEER_FILES),./peer/$(var).o) -o ./peer/main -pthread

mainT: mainT.o
	g++ $(foreach var,$(TRACKER_FILES),./tracker/$(var).o) -o ./tracker/main -pthread

mainP.o:
	$(foreach var,$(PEER_FILES),g++ -c ./peer/$(var).cpp -o ./peer/$(var).o;)

mainT.o:
	$(foreach var,$(TRACKER_FILES),g++ -c ./tracker/$(var).cpp -o ./tracker/$(var).o;)

clean:
	rm ./tracker/*.o ./tracker/main
	rm ./peer/*.o ./peer/main