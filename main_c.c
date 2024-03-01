#include "client.h"

void receivePlayerDetails(int serverSocket, struct players *details) {
	// Receive the entire struct players from the server
	debug_log(2, "Receiving Player Details");
	size_t size = sizeof(struct players);
	debuglog(2, "Receiving Player details size is ", size);
	char *ptr = (char *)details;
	size_t bytesReceived = 0;
	while (bytesReceived < size) {
		ssize_t ret = recv(serverSocket, ptr + bytesReceived, size - bytesReceived, 0);
		if (ret == -1) {
			perror("Error receiving player details");
			break;
		} else if (ret == 0) {
			// Connection closed by peer
			break;
		} else {
			bytesReceived += ret;
		}
	}
	debuglog(2, "Received Player details size is ", bytesReceived);
	debug_log(2, "Completed Receiving player details");	
}

void receiveScoreboard(int serverSocket, struct score *board) {

	// Receive the entire struct score from the server
	debug_log(2, "Receiving score board");
	size_t size = sizeof(struct score);
	debuglog(2, "Receiving Score board size is ", size);
	char *ptr = (char *)board;
	size_t bytesReceived = 0;

	while (bytesReceived < size) {
		ssize_t ret = recv(serverSocket, ptr + bytesReceived, size - bytesReceived, 0);
		if (ret == -1) {
			perror("Error receiving scoreboard");
			break;
		} else if (ret == 0) {
			// Connection closed by peer
			break;
		} else {
			bytesReceived += ret;
		}
	}
	debuglog(2, "Received score board size is ", bytesReceived);
	debug_log(2, "Completed Receiving score board");	
}

void receiveInnerValues(int serverSocket, struct inner *values) {
	// Receive the entire struct players from the server
	debug_log(2, "Receiving Inner values");
	size_t size = sizeof(struct inner);
	debuglog(2, "Receiving Inner values size is ", size);
	char *ptr = (char *)values;
	size_t bytesReceived = 0;

	while (bytesReceived < size) {
		ssize_t ret = recv(serverSocket, ptr + bytesReceived, size - bytesReceived, 0);
		if (ret == -1) {
			perror("Error receiving inner values");
			break;
		} else if (ret == 0) {
			// Connection closed by peer
			break;
		} else {
			bytesReceived += ret;
		}
	}
	debuglog(2, "Received inner values size is ", bytesReceived);
	debug_log(2, "Completed Receiving Inner values");	
}

void receiveBatterScores(int serverSocket, struct batter *scores) {
	// Receive the entire struct score from the server
	debug_log(2, "Receiving batter scores");
	size_t size = sizeof(struct batter);
	debuglog(2, "Receiving batter scores size is ", size);
	char *ptr = (char *)scores;
	size_t bytesReceived = 0;

	while (bytesReceived < size) {
		ssize_t ret = recv(serverSocket, ptr + bytesReceived, size - bytesReceived, 0);
		if (ret == -1) {
			perror("Error receiving batter scores");
			break;
		} else if (ret == 0) {
			// Connection closed by peer
			break;
		} else {
			bytesReceived += ret;
		}
	}
	debuglog(2, "Received batter scores size is ", bytesReceived);
	debug_log(2, "Completed receiving batter scores");	
}

void receiveBowlerUpdates(int serverSocket, struct bowler *updates) {
	// Receive the entire struct score from the server
	debug_log(2, "Receiving bowler updates");
	size_t size = sizeof(struct bowler);
	debuglog(2, "Receiving bowler updates size is ", size);	
	char *ptr = (char *)updates;
	size_t bytesReceived = 0;

	while (bytesReceived < size) {
		ssize_t ret = recv(serverSocket, ptr + bytesReceived, size - bytesReceived, 0);
		if (ret == -1) {
			perror("Error receiving bowler updates");
			break;
		} else if (ret == 0) {
			// Connection closed by peer
			break;
		} else {
			bytesReceived += ret;
		}
	}
	debuglog(2, "Received Bowler updates size is ", bytesReceived);
	debug_log(2, "Completed receiving bowler updates");	
}

void sendClientDetails(int clientSocket, struct Client *client_info) {
	// Calculate the size of the struct players
	debug_log(2, "Sending client details ");
	size_t size = sizeof(struct Client);
	debuglog(2, "Sending client details size is ", size);

	// Send the struct players to the client
	if (send(clientSocket, client_info, size, 0) == -1) {
		perror("Error sending client details");
	}
	debug_log(2, "completed Sending client details ");
}


int main(int argc, char *argv[]) {
	setDebugLevel(argc, argv); // Set debug level based on command-line arguments
	debug_log(2, "The program has been started");
	int clientSocket;
	struct sockaddr_in serverAddr;
	int balance = 20000;
	// Create socket
	if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error creating socket");
		exit(EXIT_FAILURE);
	}

	// Set up server address struct
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr("192.168.29.48");  // Server IP address (localhost)
	serverAddr.sin_port = htons(8080);  // Server port

	// Connect to the server
	if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
		perror("Error connecting to server");
		close(clientSocket);
		exit(EXIT_FAILURE);
	}

	printf("Connected to server.\n");

	// Inside your main function after receiving other data from the server
	receiveScoreboard(clientSocket, &board);
	receiveInnerValues(clientSocket, &values);        
	receivePlayerDetails(clientSocket, &details);
	receiveBatterScores(clientSocket, &scores);
	receiveBowlerUpdates(clientSocket, &updates); 
	displayScoreboard(&board, &updates, &scores,&values,&details);

	while (1) {
		while (1) {
			printf("Do you want to bet? (y/n): ");
			scanf(" %c", &client_info.option); // Added space before %c to consume newline character
			if (client_info.option == 'y' || client_info.option == 'Y' || client_info.option == 'N' || client_info.option == 'n' ) {
				break;
			} else {
				printf("Invalid input!\n");
			}
			while (getchar() != '\n');
		}
		if (client_info.option == 'y') {
			printf("Enter your name: ");
			scanf("%s", client_info.name);
			while (1) {
				printf("Enter on which do you want to bet for this ball\n");
				printf("0 to bet on ZERO runs\n1 to bet on ONE runs\n2 to bet on TWO runs\n3 to bet on THREE runs\n4 to bet on FOUR runs\n6 to bet on SIX runs\n7 to bet on WICKET\n");
				scanf("%d", &client_info.bet_runs);
				if (client_info.bet_runs == 0 || client_info.bet_runs == 1 || client_info.bet_runs == 2 || client_info.bet_runs == 3 || client_info.bet_runs == 4 || client_info.bet_runs == 6 ||client_info.bet_runs == 7) {
					break;
				} else {
					printf("Invalid input!\n");
				}
				while (getchar() != '\n');
			}
			while (1) {
				char s;
				int add_amount;
L:
				printf("Enter the betting amount: ");
				scanf("%d", &client_info.amount);
				if (client_info.amount > balance) {
					printf("you don't have %d in your account\n", client_info.amount);
					printf("do you want add money to your account (y|n)\n");
					scanf("  %c", &s);
					if (s == 'y') {
						printf("enter how much amount you want to add to your account\n");
						scanf("%d", &add_amount);
						balance = balance + add_amount;
					} else {
						printf("enter the betting amount less than your account balance\n");
						goto L;
					}
				} else {
					if (client_info.amount) {
						break;
					} else {
						printf("Invalid input!");
					}
					while (getchar() != '\n');
				}
			}
			balance = balance - client_info.amount;
			// Send client details to the server
			sendClientDetails(clientSocket, &client_info);
			initializeClientInfo(&client_info);
			// Inside your main function after receiving other data from the server
			receiveScoreboard(clientSocket, &board);
			receiveInnerValues(clientSocket, &values);        
			receivePlayerDetails(clientSocket, &details);
			receiveBatterScores(clientSocket, &scores);
			receiveBowlerUpdates(clientSocket, &updates); 

			// Display the received scoreboard
			displayScoreboard(&board, &updates, &scores,&values,&details);

			while (1) {
				int win_amount;
				int bytes_received = recv(clientSocket, &win_amount, sizeof(win_amount), 0);
				if (bytes_received < 0) {
					perror("Error receiving betting amount from server");
					break; // Exit the loop if there's an error
				} else if (bytes_received == 0) {
					printf("Server disconnected.\n");
					close(clientSocket);
					break; // Exit the loop if the server disconnects
				} else {
					printf("You won: %d\n", win_amount);
					balance = balance + win_amount;
					printf("total balance in your account:%d\n", balance);
					break;
				}
			}

		}
		else{
			continue;
		}
	}
	// Close the client socket
	close(clientSocket);

	return 0;
}

