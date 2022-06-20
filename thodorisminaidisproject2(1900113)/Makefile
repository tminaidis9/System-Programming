# paths
# MODULES = ../../modules
# INCLUDE = ../../include

# Αρχεία .o
OBJS = remoteClient.cpp
OBJS2 = dataServer.cpp

# Το εκτελέσιμο πρόγραμμα
EXEC = remoteClient
EXEC2 = dataServer

# Arguments we need
ARGS = -i 192.168.1.14 -p 12500 -d /
ARGS2 = -p 12500 -s 120 -q 200 -b 100


$(EXEC):$(OBJS)
	g++ -pthread -o $(EXEC) $(OBJS) 
	

$(EXEC2):$(OBJS2) 
	g++ -std=c++17 -pthread -o $(EXEC2) $(OBJS2)  

run_cl: $(EXEC)
	./$(EXEC) $(ARGS)

run_serv: $(EXEC2)
	./$(EXEC2) $(ARGS2)