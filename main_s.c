#include "server.h"

void sendPlayerDetails(int clientSocket, struct players *details) {
	// Calculate the size of the struct players
	debug_log(2, "sending player details started....");
	size_t size = sizeof(struct players);
        debuglog(2, "sending player details size is",size);
	// Send the struct players to the client
	if (send(clientSocket, details, size, 0) == -1) {
		perror("Error sending player details");
	}
	debug_log(2, "sending player details completed....");
}

void sendInnerValues(int clientSocket, struct inner *values) {
	// Calculate the size of the struct inner
	debug_log(2, "sending innier values started....");
	size_t size = sizeof(struct inner);
        debuglog(2, "sending innervalues size is",size);
	// Send the struct inner to the client
	if (send(clientSocket, values, size, 0) == -1) {
		perror("Error sending inner values");
	}
	debuglog(2, "sending innervalues size is",size);
}

void sendScoreBoard(int clientSocket, struct score *board) {
	// Calculate the size of the struct score
	debug_log(2, "sending scoreboard started....");
	size_t size = sizeof(struct score);
	debuglog(2, "sending score board size is",size);
	// Send the struct players to the client
	if (send(clientSocket, board, size, 0) == -1) {
		perror("Error sending score board");
	}
	debug_log(2, "sending scoreboard completed....");
}

void sendBowlerUpdates(int clientSocket, struct bowler *updates) {
	// Calculate the size of the struct players
	debug_log(2, "sending bowler updates started....");
	size_t size = sizeof(struct bowler);
        debuglog(2, "sending bowler updates size is",size);
	// Send the struct players to the client
	if (send(clientSocket, updates, size, 0) == -1) {
		perror("Error sending bowler updates");
	}
	debug_log(2, "sending bowler updates completed....");
}

void sendBatterScores(int clientSocket, struct batter *scores) {
	// Calculate the size of the struct players
	debug_log(2, "sending batter scores started....");
	size_t size = sizeof(struct batter);
        debuglog(2, "sending batterscores size is",size);
	// Send the struct players to the client
	if (send(clientSocket, scores, size, 0) == -1) {
		perror("Error sending batter scores");
	}
	debug_log(2, "sending batter scores updated....");
}

void receiveClientDetails(int serverSocket, struct Client *client_info) {
	// Receive the entire struct players from the server
	debug_log(2, "Started receiving the client details...");
	size_t size = sizeof(struct Client);
	debuglog(2, "Receiving structure client size is",size);
	char *ptr = (char *)client_info;
	size_t bytesReceived = 0;
	while (bytesReceived < size) {
		ssize_t ret = recv(serverSocket, ptr + bytesReceived, size - bytesReceived, 0);
		if (ret == -1) {
			perror("Error receiving client details");
			break;
		} else if (ret == 0) {
			// Connection closed by peer
			break;
		} else {
			bytesReceived += ret;
		}
	}
	debuglog(2, "Receiving structure client size is",size);
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void split_amount(void)
{     
        debug_log(2, "Entered into split amount function");
	int winnings=0,winners_count=0,split_amount,profit;
	int wickets = 0;
	if(values.wickets_taken)
	{
		wickets = 6;
	}
	printf("Number of runs scored in this ball: %d\n",values.runs_scored);
	for (int j = 0; j < client_num; j++) {
		printf("%s betted on runs : %d\n",clients[j].name,clients[j].bet_runs);
		if (clients[j].bet_runs == values.runs_scored || clients[j].bet_runs == wickets ) {
			// Client's bet matches the runs scored
			clients[j].status = 1; // Set status to indicate a win
			winners_count++; // Increase the count of winners
		} else {
			// Client's bet does not match the runs scored
			clients[j].status = 0; // Set status to indicate a loss
			winnings += clients[j].amount; // Update total winnings
		}
	}
	//gain is for profit of 10%
	profit=winnings/10;
	winnings=winnings-profit;
	// Calculate and distribute winnings
	if (winners_count > 0) {
		// Calculate split amount for each winner
		int split_amount = winnings / winners_count;
		printf("Total winnings: %d\n", winnings);
		printf("Split amount per winner: %d\n", split_amount);
		int zero=1;
		// Send winnings to each winner
		for (int j = 0; j < client_num; j++) {	
		        debuglog(2, "Started sending Scoreboard to client FD ", clients[j].fd);
			sendScoreBoard(clients[j].fd, &board);
			sendInnerValues(clients[j].fd, &values);
			sendPlayerDetails(clients[j].fd, &details);
			sendBatterScores(clients[j].fd, &scores);
			sendBowlerUpdates(clients[j].fd, &updates);
			debuglog(2, "completed sending Scoreboard to client FD ", clients[j].fd);	
			if (clients[j].status == 1) 
			{
				split_amount=clients[j].amount+split_amount;

				if(send(clients[j].fd, &split_amount, sizeof(split_amount), 0)<0)
				{
					printf("error sending\n");
				}
				else
				{
					printf("Sent split amount %d to winner client %s\n", split_amount, clients[j].name);
				}
			}
			else if(clients[j].status==0)
			{
				if(send(clients[j].fd,&zero,sizeof(zero),0)<0)
				{
					printf("error sending");
				}
				else
				{
					printf("sent 0 to the looser client\n");
				}
			}
		}
	}
	else {
		int zero=1;
		printf("No winners this time.\n");
		for (int j = 0; j < client_num; j++)
		{
			send(clients[j].fd,&zero,sizeof(zero),0);
		}

	}
	//debug_log(2, "Leaving the split amount function");	

}

// Function to handle each client connection
void *handleClient(void *arg) {
	int clientSocket = *((int *)arg);
        debug_log(2, "Entered into handle client thread");
	sendScoreBoard(clientSocket, &board);
	sendInnerValues(clientSocket, &values);
	sendPlayerDetails(clientSocket, &details);
	sendBatterScores(clientSocket, &scores);
	sendBowlerUpdates(clientSocket, &updates);

	while(1){

		receiveClientDetails(clientSocket, &new_client);
		new_client.fd=clientSocket;
		printf("Received betting updates from %s with file description ID %d\n",new_client.name,new_client.fd);
		printf("Betted on %d runs\n",new_client.bet_runs);
		printf("Betted amount is %d\n",new_client.amount);
		new_client.clientNum = -1;
		for (int i = 0; i < client_num; i++) {
			if (clients[i].fd == clientSocket) {
				new_client.clientNum = i; // Set index if client found
				printf("client_num:%d\n",new_client.clientNum);
				break;
			}
		}
		if (new_client.clientNum != -1) {
			// Update existing client
			clients[new_client.clientNum] = new_client;
			printf("Client details updated.\n");
		} else {
			// Add new client

			clients[client_num] = new_client;
			clients[client_num].clientNum = client_num;
			clients[client_num].fd = clientSocket;
			client_num++;
			printf("New client added.\n");	
		}
	}
	// Close the client socket
	close(clientSocket);
	free(arg); // Free the memory allocated for the client socket
	pthread_exit(NULL);
}
void* handleball(void* arg) {
	//int clientSocket = *((int *)arg);
	debug_log(2, "Entered into handle ball thread");
	printf("Handling ball\n");
	printf("starting the server\n");
	// Opening the SQL database
	int result = openDatabase();
	if (result != SQLITE_OK) {
		sqlite3_close(db);
	}

	// Create tables
	result = createBattersTable();
	if (result != SQLITE_OK) {
		sqlite3_close(db);
	}

	result = createBowlersTable();
	if (result != SQLITE_OK) {
		sqlite3_close(db);
	}

	result = createMatchStatisticsTable();
	if (result != SQLITE_OK) {
		sqlite3_close(db);
	}
	for (int i = 0; i < values.total_balls; i++) {

		values.runs_scored = 0;
		values.wickets_taken = 0;
		values.wide_ball = 0;
		int change;
		values.extra_ball = 0;
		int in = 0, in1 = 0;
		debug_log(2 , "timer loop started");
		while (1) {
			time(&current_time);
			if (difftime(current_time, start_time) >= 30)
			{
				break; // Stop accepting connections after 30 seconds or reaching max clients
			}
		}
		// Further processing goes here
		printf("30 seconds have passed . Proceeding with further processing...\n");


		printf("\nEnter Updates for ball %d\n", i + 1);
		printf("Enter\nG.For Updating the RUNS for GOOD ball\nN.For Updating the RUNS for NO ball\nA.For Updating the WICKETS for GOOD ball\nB.For Updating the WICKETS for NO ball\nW.For WIDE ball\nE.To EXIT\n");
		while (1) {
			scanf(" %c", &user);
			if (user == 'G' || user == 'g' || user == 'N' || user == 'n' || user == 'A' || user == 'a' || user == 'B' || user == 'b' || user == 'W' || user == 'w' || user == 'e' || user == 'E') {
				break;
			} else {
				printf("Invalid input! Please Enter valid input:");
				while (getchar() != '\n');
			}
		}
		switch (user) {
			case 'E':
			case 'e':
				//inserting into the database
				int in = 0, in1 = 0;
				if(values.innings == 1){
					for (int i = 0; i < 11; i++) {
						in = i+4;
						result = insertBatterData(&scores,&details, i, in);
						if (result != SQLITE_OK) {
							sqlite3_close(db);
						}
					}
					// Insert bowler data into bowlers table
					for (int j = 0; j < 4; j++) {
						in1 = j + 34;
						result = insertBowlerData(&updates, &details, j, in1);
						if (result != SQLITE_OK) {
							sqlite3_close(db);
						}
					}
					result = insertMatchStatistics(&board, &values);
					// Close database connection
					sqlite3_close(db);
				}
				else if(values.innings == 2){
					for (int i = 0; i < 11; i++) {
						in = i+16;
						result = insertBatterData(&scores,&details,i,in);
						if (result != SQLITE_OK) {
							sqlite3_close(db);
						}
					}
					// Insert bowler data into bowlers table
					for (int j = 0; j < 4; j++) {
						in1 = j + 28;
						result = insertBowlerData(&updates,&details,j,in1);
						if (result != SQLITE_OK) {
							sqlite3_close(db);
						}
					}
					result = insertMatchStatistics(&board, &values);
					// Close database connection
					sqlite3_close(db);
				}
				printf("Exited from the program,Thank you!");
				return 0;
				break;
			case 'g':
			case 'G':
				values.balls += 1;
				if (fabs(ball_count - 0.6) > 0.01) {
					values.overs_bowled = ball_count + overs_count;
					ball_count += 0.1;
				} else {
					values.overs_bowled = ball_count + overs_count;
					ball_count = 0.1;
					overs_count += 1;
				}
				while (1) {
					printf("Enter number for runs scored for the current ball: ");
					scanf("%d", &values.runs_scored);
					if (values.runs_scored >= 0 ) {
						break;
					} else {
						printf("Invalid input!");
					}
					while (getchar() != '\n');
				}
				break;
			case 'n':
			case 'N':
				values.extra_ball += 1;
				while (1) {
					printf("Enter number for runs scored for the current ball: ");
					scanf("%d", &values.runs_scored);
					if (values.runs_scored >= 0 ) {
						break;
					} else {
						printf("Invalid input!");
					}
					while (getchar() != '\n');
				}
				break;
			case 'A':
			case 'a':
				values.balls += 1;
				values.wickets_taken += 1;
				break;

			case 'B':
			case 'b':
				values.extra_ball += 1;
				values.wickets_taken += 1;
				break;
			case 'W':
			case 'w':
				values.wide_ball += 1;
				values.extra_ball += 1;
				break;
		}
		if(values.wide_ball || values.extra_ball)
		{
			values.total_balls += 1;
		}
		if (values.innings == 1) {
			bowlerCurrent(&details, &a, 34, 39, &values);
			// For updating the bowler statistics
			updateBowlerStats(&values,&updates);

			if (!values.wide_ball) {
				switch (values.current_striker) {
					case 0:
						strcpy(striker, batter_1);
						batterUpdatesInnings(&scores,&values, batter_1, &details);
						break;
					case 1:
						strcpy(striker, batter_2);
						batterUpdatesInnings(&scores,&values, batter_2, &details);
						break;
				}

				if (values.runs_scored == 0 && values.balls % MAX_BALLS_PER_OVER == 0) {
					values.current_striker = !values.current_striker;
				} else if (values.runs_scored > 0 && values.runs_scored % 2 != 0) {
					if (values.balls % MAX_BALLS_PER_OVER == 0) {
						values.current_striker = values.current_striker;
					} else {
						values.current_striker = !values.current_striker;
					}
				} else {
					values.current_striker = values.current_striker;
				}

				// To add the asterisk symbol to current striker
				switch (values.current_striker) {
					case 0:
						while(1){
							if (strcmp(batter_1,details.names[details.team_1_batters_start_index]) == 0){
								length = strlen(details.names[details.team_1_batters_start_index]);
								details.names[details.team_1_batters_start_index][length] = extraChar;
								details.names[details.team_1_batters_start_index][length + 1] = '\0';
								break;
							}else{
								details.team_1_batters_start_index++;
							}
						}
						break;
					case 1:
						while(1){
							if (strcmp(batter_2,details.names[details.team_1_batters_start_index]) == 0){
								length = strlen(details.names[details.team_1_batters_start_index]);
								details.names[details.team_1_batters_start_index][length] = extraChar;
								details.names[details.team_1_batters_start_index][length + 1] = '\0';
								break;
							}else{
								details.team_1_batters_start_index++;
							}
						}
						break;
				}
			}


			values.balls_rem = values.total_balls - values.balls;

			//updating the scores
			updateScoreboard(&board, &values);

			// updating the displayboard
			displayScoreboard(&board, &updates, &scores,&values,&details);
			split_amount();

			//To remove the asterisk symbol to the current batter
			length = strlen(details.names[details.team_1_batters_start_index]);
			if (length > 0) {
				details.names[details.team_1_batters_start_index][length - 1] = '\0';
				details.team_1_batters_start_index = 4;
			}

			// For changing the batsmen when wicket is down and adding extra char as out symbol
			if (values.wickets_taken == 1) {
				if (values.balls % MAX_BALLS_PER_OVER == 0) {
					switch (values.current_striker) {                          //change current striker when wicket is down at the end of the over
						case 0:

							while(1){
								if (strcmp(batter_1,details.names[details.team_1_batters_start_index]) == 0){
									length = strlen(details.names[details.team_1_batters_start_index]);
									details.names[details.team_1_batters_start_index][length] = extraOut;
									details.names[details.team_1_batters_start_index][length + 1] = '\0';
									details.team_1_batters_start_index = 4;
									break;
								}else{
									details.team_1_batters_start_index++;
								}
							}
							printf("%s's Wicket down\n", batter_1);
							details.team_1_batters_index++;
							strcpy(batter_1, details.names[details.team_1_batters_index]);
							printf("The new batsmen is %s\n", batter_1);
							values.current_striker = !values.current_striker;
							break;
						case 1:

							while(1){
								if (strcmp(batter_1,details.names[details.team_1_batters_start_index]) == 0){
									length = strlen(details.names[details.team_1_batters_start_index]);
									details.names[details.team_1_batters_start_index][length] = extraOut;
									details.names[details.team_1_batters_start_index][length + 1] = '\0';
									details.team_1_batters_start_index = 4;
									break;
								}else{
									details.team_1_batters_start_index++;
								}
							}
							printf("%s's Wicket down\n", batter_2);
							details.team_1_batters_index++;
							strcpy(batter_2, details.names[details.team_1_batters_index]);
							printf("The new batsmen is %s\n", batter_2);
							break;
					}
				} else {
					switch (values.current_striker) {              //change current striker when wicket is down
						case 0:
							printf("%s's Wicket down\n", batter_1);

							while(1){
								if (strcmp(batter_1,details.names[details.team_1_batters_start_index]) == 0){
									length = strlen(details.names[details.team_1_batters_start_index]);
									details.names[details.team_1_batters_start_index][length] = extraOut;
									details.names[details.team_1_batters_start_index][length + 1] = '\0';
									details.team_1_batters_start_index = 4;
									break;
								}else{
									details.team_1_batters_start_index++;
								}
							}

							details.team_1_batters_index++;
							strcpy(batter_1, details.names[details.team_1_batters_index]);
							printf("The new batsmen is %s\n", batter_1);
							values.wickets_taken = 0;
							break;
						case 1:
							printf("%s's Wicket down\n", batter_2);

							while(1){
								if (strcmp(batter_2,details.names[details.team_1_batters_start_index]) == 0){
									length = strlen(details.names[details.team_1_batters_start_index]);
									details.names[details.team_1_batters_start_index][length] = extraOut;
									details.names[details.team_1_batters_start_index][length + 1] = '\0';
									details.team_1_batters_start_index = 4;
									break;
								}else{
									details.team_1_batters_start_index++;
								}
							}

							details.team_1_batters_index++;
							strcpy(batter_2, details.names[details.team_1_batters_index]);
							printf("The new batsmen is %s\n", batter_2);
							values.wickets_taken = 0;
							break;
					}
				}
			}

			//innings 2
		} else if (values.innings == 2) {
			bowlerCurrent(&details, &b, 28, 33, &values);
			// For updating the bowler statistics
			updateBowlerStats(&values,&updates);
			if (!values.wide_ball) {
				switch (values.current_striker) {
					case 0:
						strcpy(striker, batter_1);
						batterUpdatesInnings(&scores,&values, batter_1, &details);
						break;
					case 1:
						strcpy(striker, batter_2);
						batterUpdatesInnings(&scores,&values, batter_2, &details);
						break;
				}

				if (values.runs_scored == 0 && values.balls % MAX_BALLS_PER_OVER == 0) {
					values.current_striker = !values.current_striker;
				} else if (values.runs_scored > 0 && values.runs_scored % 2 != 0) {
					if (values.balls % MAX_BALLS_PER_OVER == 0) {
						values.current_striker = values.current_striker;
					} else {
						values.current_striker = !values.current_striker;
					}
				} else {
					values.current_striker = values.current_striker;
				}

				// To add the asterisk symbol to current striker
				switch (values.current_striker) {
					case 0:
						while(1){
							if (strcmp(batter_1,details.names[details.team_2_batters_start_index]) == 0){
								length = strlen(details.names[details.team_2_batters_start_index]);
								details.names[details.team_2_batters_start_index][length] = extraChar;
								details.names[details.team_2_batters_start_index][length + 1] = '\0';
								break;
							}else{
								details.team_2_batters_start_index++;
							}
						}
						break;
					case 1:
						while(1){
							if (strcmp(batter_2,details.names[details.team_2_batters_start_index]) == 0){
								length = strlen(details.names[details.team_2_batters_start_index]);
								details.names[details.team_2_batters_start_index][length] = extraChar;
								details.names[details.team_2_batters_start_index][length + 1] = '\0';
								break;
							}else{
								details.team_2_batters_start_index++;
							}
						}
						break;
				}
			}
			values.balls_rem = values.total_balls - values.balls;
			//updating the scores
			updateScoreboard(&board, &values);
			// updating the displayboard
			displayScoreboard(&board, &updates, &scores,&values,&details);
			split_amount();
			//To remove the asterisk symbol to the current batter
			length = strlen(details.names[details.team_2_batters_start_index]);
			if (length > 0) {
				details.names[details.team_2_batters_start_index][length - 1] = '\0';
				details.team_2_batters_start_index = 16;
			}

			if (values.wickets_taken == 1) {
				if (values.balls % MAX_BALLS_PER_OVER == 0) {
					switch (values.current_striker) {                   //change current striker when wicket is down at the end of over
						case 0:
							while(1){
								if (strcmp(batter_1,details.names[details.team_2_batters_start_index]) == 0){
									length = strlen(details.names[details.team_2_batters_start_index]);
									details.names[details.team_2_batters_start_index][length] = extraOut;
									details.names[details.team_2_batters_start_index][length + 1] = '\0';
									details.team_1_batters_start_index = 16;
									break;
								}else{
									details.team_2_batters_start_index++;
								}
							}
							printf("%s's Wicket down\n", batter_1);
							details.team_2_batters_index++;
							strcpy(batter_1, details.names[details.team_2_batters_index]);
							printf("The new batsmen is %s\n", batter_1);
							values.current_striker = !values.current_striker;
							break;

						case 1:

							while(1){
								if (strcmp(batter_2,details.names[details.team_2_batters_start_index]) == 0){
									length = strlen(details.names[details.team_2_batters_start_index]);
									details.names[details.team_2_batters_start_index][length] = extraOut;
									details.names[details.team_2_batters_start_index][length + 1] = '\0';
									details.team_2_batters_start_index = 16;
									break;
								}else{
									details.team_2_batters_start_index++;
								}
							}
							printf("%s's Wicket down\n", batter_2);
							details.team_2_batters_index++;
							strcpy(batter_2, details.names[details.team_2_batters_index]);
							printf("The new batsmen is %s\n", batter_2);
							break;
					}
				} else {
					switch (values.current_striker) {                   //change current striker when wicket is down
						case 0:

							while(1){
								if (strcmp(batter_1,details.names[details.team_2_batters_start_index]) == 0){
									length = strlen(details.names[details.team_2_batters_start_index]);
									details.names[details.team_2_batters_start_index][length] = extraOut;
									details.names[details.team_2_batters_start_index][length + 1] = '\0';
									details.team_2_batters_start_index = 16;
									break;
								}else{
									details.team_2_batters_start_index++;
								}
							}
							printf("%s's Wicket down\n", batter_1);
							details.team_2_batters_index++;
							strcpy(batter_1, details.names[details.team_2_batters_index]);
							printf("The new batsmen is %s\n", batter_1);
							values.wickets_taken = 0;
							break;
						case 1:

							while(1){
								if (strcmp(batter_2,details.names[details.team_2_batters_start_index]) == 0){
									length = strlen(details.names[details.team_2_batters_start_index]);
									details.names[details.team_2_batters_start_index][length] = extraOut;
									details.names[details.team_2_batters_start_index][length + 1] = '\0';
									details.team_2_batters_start_index = 16;
									break;
								}else{
									details.team_2_batters_start_index++;
								}
							}
							printf("%s's Wicket down\n", batter_2);
							details.team_2_batters_index++;
							strcpy(batter_2, details.names[details.team_2_batters_index]);
							printf("The new batsmen is %s\n", batter_2);
							values.wickets_taken = 0;
							break;
					}
				}
			}

		}
		// Checking the match status
		if ((values.total_overs * MAX_BALLS_PER_OVER) == values.balls && values.innings == 1) {
			printf("\n1st innings completed to start 2nd innings\n");
			break;
		}else if ((values.total_overs * MAX_BALLS_PER_OVER) == values.balls && values.innings == 2) {
			printf("Match completed\n");
			break;
		}else if (board.wickets == 10 && values.innings == 2) {
			printf("Match completed\n");
			break;
		}else if (board.target <= board.runs && values.innings == 2) {
			printf("Match completed\n");
			break;
		} else if (board.wickets == 10 && values.innings == 1) {
			printf("1st Innings completed start 2nd innings\n");
			break;
		}

	}//while loop
	 //inserting into the database
	int in = 0, in1 = 0;
	if(values.innings == 1){
		for (int i = 0; i < 11; i++) {
			in = i+4;
			result = insertBatterData(&scores,&details, i, in);
			if (result != SQLITE_OK) {
				sqlite3_close(db);
			}
		}
		// Insert bowler data into bowlers table
		for (int j = 0; j < 4; j++) {
			in1 = j + 34;
			result = insertBowlerData(&updates, &details, j, in1);
			if (result != SQLITE_OK) {
				sqlite3_close(db);
			}
		}
		result = insertMatchStatistics(&board, &values);
		// Close database connection
		sqlite3_close(db);
	}
	else if(values.innings == 2){
		for (int i = 0; i < 11; i++) {
			in = i+16;
			result = insertBatterData(&scores,&details,i,in);
			if (result != SQLITE_OK) {
				sqlite3_close(db);
			}
		}
		// Insert bowler data into bowlers table
		for (int j = 0; j < 4; j++) {
			in1 = j + 28;
			result = insertBowlerData(&updates,&details,j,in1);
			if (result != SQLITE_OK) {
				sqlite3_close(db);
			}
		}
		result = insertMatchStatistics(&board, &values);
		// Close database connection
		sqlite3_close(db);
	}
	pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
        debug_log(2, "Entered into accept client thread");
	setDebugLevel(argc, argv); 
	int serverSocket;
	struct sockaddr_in serverAddr;
	pthread_t threads[MAX_CLIENTS];
	int numClients = 0;
	// Create socket
	if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error creating socket");
		exit(EXIT_FAILURE);
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY; 
	serverAddr.sin_port = htons(8080);   

	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
		perror("Error binding socket");
		close(serverSocket);
		exit(EXIT_FAILURE);
	}

	if (listen(serverSocket, 5) == -1) {
		perror("Error listening");
		close(serverSocket);
		exit(EXIT_FAILURE);
	}
	printf("Server listening on port 8080...\n");

	initializeScoreboard(&board);
	initializeInnerValues(&values);
	initializePlayersDetails(&details);
	initializeBatter(&scores);
	initializeBowler(&updates);

	FILE *file;
	int i;
	file = fopen("cricket_data.txt", "r");
	if (file == NULL) {
		perror("Error opening file");
		return 1; 
	}

	for (i = 0; i < MAX_NAMES; i++) {
		if (fgets(details.names[i], MAX_NAME_LENGTH, file) == NULL) {
			break; 
		}
		if (details.names[i][strlen(details.names[i]) - 1] == '\n') {
			details.names[i][strlen(details.names[i]) - 1] = '\0';
		}
	}
	fclose(file);
        debug_log(2 , "Player details read from the file");
	printf("Enter total no of overs for this Match: ");
	while (1) {
		scanf("%d", &values.total_overs);
		if (values.total_overs > 0) {
			break;
		} else {
			printf("Invalid input! Please Enter valid input: ");
			while (getchar() != '\n');
		}
	}

	printf("Enter 1 for FIRST innings and 2 for SECOND innings:");
	while (1) {
		scanf("%d", &values.innings);

		if (values.innings == 1 || values.innings == 2) {
			break;
		} else {
			printf("\nInvalid input! Please enter a valid input: ");
			while (getchar() != '\n');
		}
	}

	values.total_balls = MAX_BALLS_PER_OVER * values.total_overs;
	if (values.innings == 2) {
		printf("Enter target score: ");
		while (1) {
			scanf("%d", &board.target);
			if (board.target >= 0) {
				break;
			} else {
				printf("Invalid input! Please Enter valid input: ");
				while (getchar() != '\n');
			}
		}
	}
	if (values.innings == 1) {
		extraChar = '*';
		length = strlen(details.names[a]);
		details.names[details.team_2_bowlers_index][length] = extraChar;
		details.names[details.team_2_bowlers_index][length + 1] = '\0';
		strcpy(batter_1, details.names[details.team_1_batters_index]);
		details.team_1_batters_index++;
		strcpy(batter_2, details.names[details.team_1_batters_index]);
	}
	if (values.innings == 2) {
		extraChar = '*';
		length = strlen(details.names[details.team_1_bowlers_index]);
		details.names[details.team_1_bowlers_index][length] = extraChar;
		details.names[details.team_1_bowlers_index][length + 1] = '\0';
		strcpy(batter_1, details.names[details.team_2_batters_index]);
		details.team_2_batters_index++;
		strcpy(batter_2, details.names[details.team_2_batters_index]);
	}

	// Accept and handle incoming connections
	while (1) {
		struct sockaddr_in clientAddr;
		socklen_t clientAddrLen = sizeof(clientAddr);
		int *clientSocket = malloc(sizeof(int)); // Allocate memory for the client socket

		// Accept incoming connection
		if ((*clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen)) == -1) {
			perror("Error accepting connection");
			close(serverSocket);
			free(clientSocket); // Free the memory allocated for the client socket
			exit(EXIT_FAILURE);
		}

		printf("Client connected.\n");

		pthread_t client_tid, ball_tid;
		if (pthread_create(&client_tid, NULL, handleClient, (void *)clientSocket) != 0) {
			perror("Error creating client thread");
			close(*clientSocket);
			free(clientSocket);
			exit(EXIT_FAILURE);
		}
		pthread_detach(client_tid);

		if (pthread_create(&ball_tid, NULL, handleball,NULL) != 0) {
			perror("Error creating ball thread");
			exit(EXIT_FAILURE);
		}
		pthread_detach(ball_tid);

	}

	// Close the server socket (This part of the code is never reached in this example)
	close(serverSocket);

	return 0;
}
