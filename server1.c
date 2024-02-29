#include "include_structure.h"
struct Client clients[MAX_CLIENTS];
int client_num=0;
char user;
char buf[]="hello";
 int server_fd, new_socket, valread;
      struct sockaddr_in address;

      int opt = 1;
      int addrlen = sizeof(address);  
      time_t start_time, current_time;
      pthread_t threads[MAX_CLIENTS];
      pthread_t threads1[MAX_CLIENTS];
      int thread_count = 0;
// SQLite database and statement variables
sqlite3 *db;
sqlite3_stmt *stmt;

// Function to open the SQLite database
int openDatabase() {
	// Open database file
	int result = sqlite3_open("cricket.db", &db);
	if (result != SQLITE_OK) {
		// Print error message if failed to open
		fprintf(stderr, "Error opening database: %s\n", sqlite3_errmsg(db));
		return result;
	}
	return SQLITE_OK;
}

// Function to create the batters table in the database
int createBattersTable() {
	// SQL statement to create batters table
	const char *sql = "CREATE TABLE IF NOT EXISTS batters ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"name TEXT,"
		"runs INTEGER,"
		"balls INTEGER,"
		"fours INTEGER,"
		"sixes INTEGER,"
		"strike_rate REAL"
		");";

	// Execute SQL statement
	int result = sqlite3_exec(db, sql, 0, 0, 0);
	if (result != SQLITE_OK) {
		// Print error message if failed to create table
		fprintf(stderr, "Error creating batters table: %s\n", sqlite3_errmsg(db));
		return result;
	}
	return SQLITE_OK;
}

// Function to create the bowlers table in the database
int createBowlersTable() {
	// SQL statement to create bowlers table
	const char *sql = "CREATE TABLE IF NOT EXISTS bowlers ("
		"id INTEGER PRIMARY KEY AUTOINCREMENT,"
		"name TEXT,"
		"overs INTEGER,"
		"maidens INTEGER,"
		"runs_given INTEGER,"
		"economy REAL"
		");";

	// Execute SQL statement
	int result = sqlite3_exec(db, sql, 0, 0, 0);
	if (result != SQLITE_OK) {
		// Print error message if failed to create table
		fprintf(stderr, "Error creating bowlers table: %s\n", sqlite3_errmsg(db));
		return result;
	}
	return SQLITE_OK;
}

// Function to create the MatchStatistics table in the database
int createMatchStatisticsTable() {
	// SQL statement to create MatchStatistics table
	const char *sql = "CREATE TABLE IF NOT EXISTS MatchStatistics ("
		"MatchID INTEGER PRIMARY KEY AUTOINCREMENT,"
		"TotalOvers INT,"
		"Runs INT,"
		"Target INT,"
		"Wickets INT,"
		"BallsRemaining INT"
		");";

	// Execute SQL statement
	int result = sqlite3_exec(db, sql, 0, 0, 0);
	if (result != SQLITE_OK) {
		// Print error message if failed to create table
		fprintf(stderr, "Error creating MatchStatistics table: %s\n", sqlite3_errmsg(db));
		return result;
	}
	return SQLITE_OK;
}

// Function to insert batter data into batters table
int insertBatterData(struct batter *scores, struct players *details, int i, int in) {
	// SQL statement to insert batter data
	const char *sql = "INSERT INTO batters (name, runs, balls, fours, sixes, strike_rate) VALUES (?, ?, ?, ?, ?, ?);";
	int result = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (result != SQLITE_OK) {
		// Print error message if failed to prepare statement
		fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
		return result;
	}

	// Bind parameters to SQL statement
	sqlite3_bind_text(stmt, 1, details->names[in], -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, scores->batter_runs[i]);
	sqlite3_bind_int(stmt, 3, scores->batter_balls[i]);
	sqlite3_bind_int(stmt, 4, scores->batter_fours[i]);
	sqlite3_bind_int(stmt, 5, scores->batter_sixes[i]);
	sqlite3_bind_double(stmt, 6, scores->batter_strike[i]);

	// Execute SQL statement
	result = sqlite3_step(stmt);
	if (result != SQLITE_DONE) {
		// Print error message if failed to insert data
		fprintf(stderr, "Error inserting data into batters table: %s\n", sqlite3_errmsg(db));
		return result;
	}

	sqlite3_finalize(stmt);
	return SQLITE_OK;
}

// Function to insert bowler data into bowlers table
int insertBowlerData(struct bowler *updates, struct players *details, int j, int in1) {
	// SQL statement to insert bowler data
	const char *sql = "INSERT INTO bowlers (name, overs, maidens, runs_given, economy) VALUES (?, ?, ?, ?, ?);";
	int result = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (result != SQLITE_OK) {
		// Print error message if failed to prepare statement
		fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
		return result;
	}

	// Bind parameters to SQL statement
	sqlite3_bind_text(stmt, 1, details->names[in1], -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 2, updates->overs_bowled[j]);
	sqlite3_bind_int(stmt, 3, updates->bowler_maiden[j]);
	sqlite3_bind_int(stmt, 4, updates->bowler_runs[j]);
	sqlite3_bind_double(stmt, 5, updates->economy[j]);

	// Execute SQL statement
	result = sqlite3_step(stmt);
	if (result != SQLITE_DONE) {
		// Print error message if failed to insert data
		fprintf(stderr, "Error inserting data into bowlers table: %s\n", sqlite3_errmsg(db));
		return result;
	}

	sqlite3_finalize(stmt);
	return SQLITE_OK;
}

// Function to insert match statistics into MatchStatistics table
int insertMatchStatistics(struct score *board, struct inner *values) {
	// SQL statement to insert match statistics
	const char *sql = "INSERT INTO MatchStatistics (TotalOvers, Runs, Target, Wickets, BallsRemaining) VALUES (?, ?, ?, ?, ?);";
	int result = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
	if (result != SQLITE_OK) {
		// Print error message if failed to prepare statement
		fprintf(stderr, "Error preparing statement: %s\n", sqlite3_errmsg(db));
		return result;
	}

	// Bind parameters to SQL statement
	sqlite3_bind_int(stmt, 1, values->total_overs);
	sqlite3_bind_int(stmt, 2, board->runs);
	sqlite3_bind_int(stmt, 3, board->target);
	sqlite3_bind_int(stmt, 4, board->wickets);
	sqlite3_bind_int(stmt, 5, values->balls_rem);

	// Execute SQL statement
	result = sqlite3_step(stmt);
	if (result != SQLITE_DONE) {
		// Print error message if failed to insert data
		fprintf(stderr, "Error inserting data into MatchStatistics table: %s\n", sqlite3_errmsg(db));
		return result;
	}

	sqlite3_finalize(stmt);
	return SQLITE_OK;
}
// Function to initialize the scoreboard
void initializeScoreboard(struct score *board) {
	// Initialize all scoreboard variables to zero
	board->runs = 0;             // Total runs scored
	board->target = 0;           // Target runs for the chasing team
	board->overs = 0.0;          // Total overs bowled
	board->wickets = 0;          // Total wickets fallen
	board->req_runs = 0;         // Required runs for the chasing team
	board->run_rate = 0.0;       // Current run rate
	board->req_run_rate = 0.0;   // Required run rate for the chasing team
	board->extra_balls = 0;      // Extra balls bowled
}

// Function to initialize inner match details
void initializeInnerValues(struct inner *values) {
	values->runs_scored = 0;        // Runs scored in the current ball
	values->overs_bowled = 0.0;     // Overs bowled in the current ball
	values->wickets_taken = 0;      // Wickets taken in the current ball
	values->total_overs = 0;        // Total overs in the match
	values->innings = 0;            // Current innings number
	values->total_balls = 0;        // Total balls bowled
	values->wide_ball = 0;          // Count of wide balls
	values->extra_ball = 0;         // Count of extra balls
	values->balls_rem = 0;          // Remaining balls in the current over
	values->current_striker = 0;    // Index of the current striker
	values->balls = 0;              // Balls bowled in the current over
}

// Function to initialize player details
void initializePlayersDetails(struct players *details) {
	// Initialize player details indices
	details->team_1_name_index = 1;
	details->team_2_name_index = 2;
	details->team_1_batters_index = 4;
	details->team_2_batters_index = 16;
	details->team_1_bowlers_index = 28;
	details->team_2_bowlers_index = 34;
	details->team_1_batters_start_index = 4;
	details->team_1_batters_stop_index = 14;
	details->team_2_batters_start_index = 16;
	details->team_2_batters_stop_index = 26;
	details->umpire_1_index = 40;
	details->umpire_2_index = 41;
	details->umpire_3_index = 42;
}

// Function to initialize batter details
void initializeBatter(struct batter *scores) {
	// Initialize batter details for each player
	for (int i = 0; i < 11; i++) {
		scores->batter_runs[i] = 0;    // Runs scored by each batter
		scores->batter_balls[i] = 0;    // Balls faced by each batter
		scores->batter_sixes[i] = 0;    // Sixes hit by each batter
		scores->batter_fours[i] = 0;    // Fours hit by each batter
		scores->batter_strike[i] = 0.0; // Strike rate of each batter
	}
}

// Function to initialize bowler details
void initializeBowler(struct bowler *updates) {
	// Initialize bowler details for each player
	for (int i = 0; i < 5; i++) {
		updates->bowler_balls[i] = 0;          // Balls bowled by each bowler
		updates->bowler_maiden[i] = 0;         // Maidens bowled by each bowler
		updates->bowler_over1[i] = 0;          // Runs conceded in first over by each bowler
		updates->bowler_over2[i] = 0;          // Runs conceded in second over by each bowler
		updates->bowler_runs[i] = 0;           // Total runs conceded by each bowler
		updates->bowler_wickets[i] = 0;        // Wickets taken by each bowler
		updates->bowler_over_runs[i] = 0;      // Runs conceded in the current over by each bowler
		updates->overs_bowled[i] = 0.0;        // Overs bowled by each bowler
		updates->economy[i] = 0.0;             // Economy rate of each bowler
		updates->bowler_change = 0;            // Flag indicating bowler change
	}
}

// Function to update the scoreboard based on the current ball
void updateScoreboard(struct score *board, struct inner *values) {
	// Update runs, overs, and wickets
	debug_log(2 , "Entered into the updatescoreboard function");
	board->runs = board->runs + values->runs_scored + values->wide_ball;  // Total runs
	board->overs = values->overs_bowled;                                 // Total overs
	board->wickets += values->wickets_taken;                             // Total wickets
	board->extra_balls += values->extra_ball;                            // Extra balls bowled
									     // Calculate required runs and run rate for the second innings
	if (values->innings == 2) {
		board->req_runs = board->target - board->runs;                   // Required runs
		board->req_run_rate = board->req_runs / (values->total_overs - board->overs); // Required run rate
	} else {
		// Reset values for the first innings
		board->req_runs = 0;
		board->req_run_rate = 0;
	}
	// Calculate the current run rate
	board->run_rate = board->runs / board->overs;
	debug_log(2 , "Leaving from updatescoreboard function");
}
// Function to update the current bowler's name with an extra character
void bowlerCurrent(struct players *details, int *index, int startIndex, int stopIndex, struct inner *values) {
	int length;
	char extraChar;
        debug_log(2 , "Entered into the bowlercurrent function");
	// Check if the current over is completed
	if (values->balls % MAX_BALLS_PER_OVER == 0) {
		// Remove the last character from the current bowler's name
		length = strlen(details->names[*index]);
		if (length > 0) {
			details->names[*index][length - 1] = '\0';
		}
		extraChar = '*'; // Extra character to mark the current bowler
		++*index; // Move to the next bowler
		length = strlen(details->names[*index]);
		// Add the extra character at the end of the new bowler's name
		details->names[*index][length] = extraChar;
		details->names[*index][length + 1] = '\0';
		// If all bowlers have bowled their overs, mark the first bowler with the extra character too
		if (*index == stopIndex) {
			*index = startIndex;
			length = strlen(details->names[*index]);
			details->names[*index][length] = extraChar;
			details->names[*index][length + 1] = '\0';
		}
	}
	debug_log(2 , "Leaving bowlercurrent function");
}

// Function to update bowler statistics after each ball
void updateBowlerStats(struct inner *values, struct bowler *updates) {
	// If it's a regular ball (not wide or extra)
	debug_log(2 , "Entered into the updatebowlerstats function");
	if (values->wide_ball == 0 && values->extra_ball == 0) {
		// Update balls, runs, over runs, overs, and economy
		updates->bowler_balls[updates->bowler_change] += 1;
		updates->bowler_runs[updates->bowler_change] += values->runs_scored;
		updates->bowler_over_runs[updates->bowler_change] += values->runs_scored;
		updates->bowler_over1[updates->bowler_change] = updates->bowler_balls[updates->bowler_change] / MAX_BALLS_PER_OVER;
		updates->bowler_over2[updates->bowler_change] = updates->bowler_balls[updates->bowler_change] % MAX_BALLS_PER_OVER;
		// If a wicket is taken, update wickets
		if (values->wickets_taken) {
			(updates->bowler_wickets[updates->bowler_change]) += 1;
		}
		// If the over is completed
		if (updates->bowler_balls[updates->bowler_change] % MAX_BALLS_PER_OVER == 0) {
			// If it's a maiden over (no runs scored)
			if (updates->bowler_over_runs[updates->bowler_change] == 0) {
				(updates->bowler_maiden[updates->bowler_change]) += 1;
				(updates->bowler_over_runs[updates->bowler_change]) = 0;
			}
			// Update overs bowled and economy
			updates->overs_bowled[updates->bowler_change] += 1;
			updates->economy[updates->bowler_change] = updates->bowler_runs[updates->bowler_change] / updates->overs_bowled[updates->bowler_change];
			updates->bowler_change += 1;  // Increment bowler_change when the over is completed
		}
	}
	// If it's a wide ball, only update runs
	else if (values->wide_ball) {
		(updates->bowler_runs[updates->bowler_change]) += 1;
		(updates->bowler_over_runs[updates->bowler_change]) += 1;
	}
	// If it's an extra ball, update runs and wickets if taken
	else if (values->extra_ball) {
		(updates->bowler_runs[updates->bowler_change]) += values->runs_scored;
		(updates->bowler_over_runs[updates->bowler_change]) += values->runs_scored;
		if (values->wickets_taken) {
			(updates->bowler_wickets[updates->bowler_change]) += 1;
		}
	}
	// Reset bowler_change when all bowlers have bowled their overs
	if (updates->bowler_change == 5) {
		updates->bowler_change = 0;
	}
	debug_log(2 , "Leaving updatebowlerstats function");
}

// Function to update scores of a particular batter for each innings
void batterUpdatesInnings(struct batter *scores, struct inner *values, char batter[], struct players *details) {
	int index = 0, current = 0;
	debug_log(2 , "Entered into batterupdatesInnings function");
	if (values->innings == 1) {
		index = 4; // Starting index for the first innings
		current = 0; // Current batter index
	} else if (values->innings == 2) { // Check if it's the second innings
		index = 16; // Starting index for the second innings
		current = 0; // Current batter index
	}

	while (1) {
		if (!strcmp(batter, details->names[index])) {
			scores->batter_balls[current] += 1;
			scores->batter_runs[current] += values->runs_scored;
			scores->batter_strike[current] = (scores->batter_runs[current] * 100) / scores->batter_balls[current];
			if (values->runs_scored == 4) {
				scores->batter_fours[current] += 1; // Increment the number of fours
			} else if (values->runs_scored == MAX_BALLS_PER_OVER) { // Check if the runs scored by the batter is a six
				scores->batter_sixes[current] += 1; // Increment the number of sixes
			}
			break; // Exit the loop
		} else {
			index++; // Move to the next batter
			current++; // Move to the next index for tracking current batter
		}
	}
	debug_log(2 , "Leaving from batterupdatesInnings function");
}

void sendPlayerDetails(int clientSocket, struct players *details) {
	// Calculate the size of the struct players
	debug_log(2, "sending player details started....");
	size_t size = sizeof(struct players);
	debuglog(2, "sending player details size is",size);
	//printf("client socket:%d\n",clientSocket);
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
        //printf("client socket:%d\n",clientSocket);
	// Send the struct inner to the client
	if (send(clientSocket, values, size, 0) == -1) {
		perror("Error sending inner values");
	}
	  debug_log(2, "sending innier values completed....");
}

void sendScoreBoard(int clientSocket, struct score *board) {
	// Calculate the size of the struct score
	debug_log(2, "sending scoreboard started....");
	size_t size = sizeof(struct score);
	debuglog(2, "sending score board size is",size);
         //printf("client socket:%d\n",clientSocket);
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
        //printf("client socket:%d\n",clientSocket);
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
        //printf("client socket:%d\n",clientSocket);
	// Send the struct players to the client
	if (send(clientSocket, scores, size, 0) == -1) {
		perror("Error sending batter scores");
	}
	debug_log(2, "sending batter scores updated....");
}

/*void sendPlayerNames(int clientSocket,struct names *names)
{
	size_t size=sizeof(struct names);
	if(send(clientSocket,names,size,0) == -1)
	{
		perror("error sending names");
	}
}*/

// Function to set debug level based on command-line arguments
void setDebugLevel(int argc, char *argv[]) {
    if (argc <= 3) {
        if (argc == 1) {
            printf("\n");
        } else if (argc == 2) {
            if (!strcmp(argv[1], "-v") == 0) {
                printf("Invalid input! Enter -v\n");
                exit(1);
            } else {
                DEBUG_LEVEL = 2;
                printf("Debug level set to: %d\n", DEBUG_LEVEL);
            }
        } else if (argc == 3) {
            if (!strcmp(argv[1], "-v") && (strcmp(argv[2], "1") == 0 || strcmp(argv[2], "2") == 0 || strcmp(argv[2], "3") == 0)) {
                DEBUG_LEVEL = atoi(argv[2]);
                printf("Debug level set to: %d\n", DEBUG_LEVEL);
            } else {
                printf("Invalid input!\n Input format is -v 1 or -v 2 or -v 3 only\n");
                exit(1);
            }
        } else {
            DEBUG_LEVEL = 2;
            printf("Debug level set to: %d\n", DEBUG_LEVEL);
        }
    } 
}
void debug_log(int level,  const char * message) {
  if (debug >= level) {
    printf("Debug[%d]: %s\n", level, message);
  }
}
void debuglog(int level,  const char * message, size_t size) {
  if (debug >= level) {
    printf("Debug[%d]: %s %ld\n", level, message,size);
  }
}

void *handle_client(void *arg) {
debug_log(2, "Entered into handle client thread");
	int client_socket = *((int *)arg);
	struct Client new_client;
	char ackn[10];
	ssize_t bytes_received;
	time_t start_time, current_time;
	// Get the start time
	time(&start_time); 
	// Receive client details within 30 seconds
	while(1)
	{
	        recv(client_socket,ackn,sizeof(ackn),0);
	        debug_log(2, "Received ack from client");
	        if(strcmp(ackn,"ack")==0)
	        {
	        sendScoreBoard(client_socket, &board);
		sendInnerValues(client_socket, &values);
		sendPlayerDetails(client_socket, &details);
		sendBatterScores(client_socket, &scores);
		sendBowlerUpdates(client_socket, &updates);
		}
		ssize_t bytes_received = recv(client_socket, &new_client, sizeof(new_client), 0);
		if (bytes_received < 0) {
			perror("recv");
			break;
		} else if (bytes_received == 0) {
			printf("Client disconnected.\n");
			break;
		} else {
		debug_log(2, "Received client details from the server");
			// Print received client details
			printf("Client details received:\n");
			printf("Name: %s\n", new_client.name);
			printf("Bet runs: %d\n", new_client.bet_runs);
			printf("Amount: %d\n", new_client.amount);
			printf("option:%c\n",new_client.option);
			new_client.fd=client_socket;
			printf("fd is:%d\n",new_client.fd);
			new_client.clientNum = -1; // Initialize to a value indicating not found
            // Check if client already exists
            for (int i = 0; i < client_num; i++) {
                if (clients[i].fd == client_socket) {
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
                    clients[client_num].fd = client_socket;
                    client_num++;
                    printf("New client added.\n");	
		}
		sleep(1);
	}
	}
	debug_log(2, "Exiting handle client thread");
	pthread_exit(NULL);
}
void *accept_client(void *arg)
{          debug_log(2, "Entered into accept client thread");
           int server_fd=*((int *)arg);
           int new_socket;
                     while(1)
                     {
                     
                      if(strcmp(buf,"hello")==0)
                      {
                      printf("buffer in accept client%s\n:",buf);
		      if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
			      perror("accept");
		      }
		      
		      printf("New client connected\n");
		      if(new_socket!=0)
		      {
			      if (pthread_create(&threads[thread_count], NULL, handle_client, &new_socket) != 0) {
				      perror("pthread_create");
				      close(new_socket);
			      }

			      thread_count++;
			      printf("client_num:%d",client_num);
		      }
		      }
		      else
		      {
		      continue;
		      }
		      }
		      pthread_exit(NULL);
}
void split_amount(void)
{       debug_log(2, "Entered into split amount function");
	int winnings=0,winners_count=0,split_amount,profit;
	char ack[20]="acknowledge";
	printf("runs scored %d\n",values.runs_scored);
	for (int j = 0; j < client_num; j++) {
	        if(clients[j].choice==user)
	        {
			if(clients[j].choice == 'g' ||clients[j].choice == 'G')
			{
				if (clients[j].bet_runs == values.runs_scored)
				{
					clients[j].status = 1; // Set status to indicate a win
					winners_count++; // Increase the count of winners
			        }
				else {
					// Client's bet does not match the runs scored
					clients[j].status = 0; // Set status to indicate a loss
					winnings += clients[j].amount; // Update total winnings
				}
			}
			else if(clients[j].choice == 'n' ||clients[j].choice == 'N')
			{
				if (clients[j].bet_runs == values.runs_scored)
				{
				        clients[j].status = 1; // Set status to indicate a win
					winners_count++; // Increase the count of winners
				}
				else {
					// Client's bet does not match the runs scored
					clients[j].status = 0; // Set status to indicate a loss
					winnings += clients[j].amount; // Update total winnings
				}
			}
			else if(clients[j].choice == 'a' ||clients[j].choice == 'A')
			{
			        clients[j].status = 1; // Set status to indicate a win
				winners_count++;
			}
			else if(clients[j].choice == 'b' ||clients[j].choice == 'B')
			{
			        clients[j].status = 1; // Set status to indicate a win
				winners_count++;
			}
			else if(clients[j].choice == 'w' ||clients[j].choice == 'W')
			{
			        clients[j].status = 1; // Set status to indicate a win
				winners_count++;
			}
		}
	  else
		{
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
			char arr[20]="acknowledge";
			send(clients[j].fd,&arr,sizeof(arr),0);
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
			send(clients[j].fd,&ack,sizeof(ack),0);
			send(clients[j].fd,&zero,sizeof(zero),0);
		}

	   }
	debug_log(2, "Leaving the split amount function");	
	strcpy(buf,"hello");

}

int main(int argc,char * argv[]) {

	float ball_count = 0.1, overs_count = 0.0;
	char striker[30], batter_1[30], batter_2[30];
	char extraChar;
	int a = 34, b = 28, length = 0;
	char extraOut = '~';
	setDebugLevel(argc, argv); // Set debug level based on command-line arguments
        debug_log(2 , "The program has been started");
	initializeBatter(&scores);
	initializeInnerValues(&values);
	initializePlayersDetails(&details);
	initializeBowler(&updates);
	initializeScoreboard(&board);
	// Opening text file
	FILE *file;
	int i;
	file = fopen("cricket_data.txt", "r");
	if (file == NULL) {
		perror("Error opening file");
		return 1; // Exit with an error code
	}

	// Reading names from the text file
	for (i = 0; i < MAX_NAMES; i++) {
		if (fgets(details.names[i], MAX_NAME_LENGTH, file) == NULL) {
			break; // Break the loop if end of file is reached or an error occurs
		}

		// Remove the newline character at the end of each name
		if (details.names[i][strlen(details.names[i]) - 1] == '\n') {
			details.names[i][strlen(details.names[i]) - 1] = '\0';
		}
		debug_log(2 , "Player details read from the file");
	}
	fclose(file);

	// Opening the SQL database
	int result = openDatabase();
	if (result != SQLITE_OK) {
		sqlite3_close(db);
		return 1;
	}

	// Create tables
	result = createBattersTable();
	if (result != SQLITE_OK) {
		sqlite3_close(db);
		return 1;
	}

	result = createBowlersTable();
	if (result != SQLITE_OK) {
		sqlite3_close(db);
		return 1;
	}

	result = createMatchStatisticsTable();
	if (result != SQLITE_OK) {
		sqlite3_close(db);
		return 1;
	}

	// Input from user
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
l1:   if (values.innings == 2) {
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
      //updating the scores
      updateScoreboard(&board,&values);

      // updating the displayboard
      displayScoreboard(&board, &updates, &scores,&values,&details);

      // Create a TCP socket
      if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
	      perror("socket failed");
	      exit(EXIT_FAILURE);
      }

      // Set socket options to allow multiple connections
      if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
	      perror("setsockopt");
	      exit(EXIT_FAILURE);
      }

      address.sin_family = AF_INET;
      address.sin_addr.s_addr =inet_addr("192.168.29.239");
      address.sin_port = htons(PORT);

      // Bind the socket to localhost and the specified port
      if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
	      perror("bind failed");
	      exit(EXIT_FAILURE);
      }
      
      // Listen for incoming connections
      if (listen(server_fd, MAX_CLIENTS) < 0) {
	      perror("listen");
	      exit(EXIT_FAILURE);
      } 
      pthread_t p;
       if (pthread_create(&p, NULL,accept_client, &server_fd) != 0)
       {
	     perror("pthread_create");
	     close(server_fd);
       }

      for (int i = 0; i < values.total_balls; i++) {
	      values.runs_scored = 0;
	      values.wickets_taken = 0;
	      values.wide_ball = 0;
	      int change;
	      values.extra_ball = 0;
	      int in = 0, in1 = 0,f;
	      time(&start_time); // Get current time

	      printf("Server is listening on port %d\n", PORT);
	      while (1) {
	              debug_log(2 , "Entered into the timer loop");
		      time(&current_time);
		      if (difftime(current_time, start_time) >= 30)
		      {
		             strcpy(buf,"No");
			      break; // Stop accepting connections after 30 seconds or reaching max clients
                       }
	      }
	      // Further processing goes here
	      printf("30 seconds have passed . Proceeding with further processing...\n");

	      printf("\nEnter Updates for ball %d\n", i + 1);
	      printf("Enter\nG.For Updating the RUNS for GOOD ball\nN.For Updating the RUNS for NO ball\nA.For Updating the WICKETS for GOOD ball\nB.For Updating the WICKETS for NO ball\nW.For WIDE ball\nE.To EXIT\n");

	      while (1) {
		      scanf("  %c", &user);
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
			      if(values.innings == 1){
				      for (int i = 0; i < 11; i++) {
					      in = i+4;
					      result = insertBatterData(&scores,&details,i,in);
					      if (result != SQLITE_OK) {
						      sqlite3_close(db);
						      return 1;
					      }
				      }
				      // Insert bowler data into bowlers table
				      for (int j = 0; j < 4; j++) {
					      in1 = j + 34;
					      result = insertBowlerData(&updates,&details,j,in1);
					      if (result != SQLITE_OK) {
						      sqlite3_close(db);
						      return 1;
					      }
				      }
				      result = insertMatchStatistics(&board,&values);
				      // Close database connection
				      sqlite3_close(db);
			      }
			      else if(values.innings == 2){
				      for (int i = 0; i < 11; i++) {
					      in = i+16;
					      result = insertBatterData(&scores,&details,i,in);
					      if (result != SQLITE_OK) {
						      sqlite3_close(db);
						      return 1;
					      }
				      }
				      // Insert bowler data into bowlers table
				      for (int j = 0; j < 4; j++) {
					      in1 = j + 28;
					      result = insertBowlerData(&updates,&details,j,in1);
					      if (result != SQLITE_OK) {
						      sqlite3_close(db);
						      return 1;
					      }
				      }
				      result = insertMatchStatistics(&board,&values);
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
				      f=values.runs_scored;
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
	      split_amount();




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
		      updateScoreboard(&board,&values);

		      // updating the displayboard
		      displayScoreboard(&board, &updates, &scores,&values,&details);

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

      }

      //inserting into the database
      int in = 0, in1 = 0;
      if(values.innings == 1){
	      for (int i = 0; i < 11; i++) {
		      in = i+4;
		      result = insertBatterData(&scores,&details, i, in);
		      if (result != SQLITE_OK) {
			      sqlite3_close(db);
			      return 1;
		      }
	      }
	      // Insert bowler data into bowlers table
	      for (int j = 0; j < 4; j++) {
		      in1 = j + 34;
		      result = insertBowlerData(&updates, &details, j, in1);
		      if (result != SQLITE_OK) {
			      sqlite3_close(db);
			      return 1;
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
			      return 1;
		      }
	      }
	      // Insert bowler data into bowlers table
	      for (int j = 0; j < 4; j++) {
		      in1 = j + 28;
		      result = insertBowlerData(&updates,&details,j,in1);
		      if (result != SQLITE_OK) {
			      sqlite3_close(db);
			      return 1;
		      }
	      }
	      result = insertMatchStatistics(&board, &values);
	      // Close database connection
	      sqlite3_close(db);
      }
      // Wait for all threads to finish
      for (int i = 0; i < thread_count; i++) {
	      pthread_join(threads[i], NULL);
      }

      return 0;
}

