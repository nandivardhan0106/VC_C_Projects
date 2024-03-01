#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h> 
#include <sqlite3.h>

#define MAX_NAMES 100
#define MAX_NAME_LENGTH 1000
#define PORT 8080
#define MAX_CLIENTS 100
#define MAX_BALLS_PER_OVER 6

float ball_count = 0.1;
float overs_count = 0.0;
char extraChar = '*';
int a = 34;
int b = 28;
int length = 0;
char extraOut = '~';
char striker[30];
char batter_1[30];
char batter_2[30];
char user;
int DEBUG_LEVEL = 0; // Global variable to hold debug level
time_t start_time, current_time;
int client_num=0;


void Debuglog1(int level, const char *message);
void Debuglog2(int level, const char *message, size_t size);
void Debuglog3(int level, const char *message);

// Structure to represent the cricket scoreboard
struct score {
	int target;
	int runs;
	float overs;
	int wickets;
	int req_runs;
	float run_rate;
	float req_run_rate;
	int total;
	int extra_balls;
}board;
struct names
{
	char names[60][100];
}names;
// Structure to update the inner values
struct inner {
	int runs_scored;
	float overs_bowled;
	int wickets_taken;
	int total_overs;
	int innings;
	int total_balls;
	int wide_ball;
	int extra_ball;
	int balls_rem;
	int current_striker;
	int balls;
}values;

// Structure to store player details
struct players {
	char names[MAX_NAMES][MAX_NAME_LENGTH];
	int team_1_name_index;
	int team_2_name_index;
	int team_1_batters_index;
	int team_2_batters_index;
	int team_1_bowlers_index;
	int team_2_bowlers_index;
	int team_1_batters_start_index;
	int team_1_batters_stop_index;
	int team_2_batters_start_index;
	int team_2_batters_stop_index;
	int umpire_1_index;
	int umpire_2_index;
	int umpire_3_index;
}details;

// Structure to store batter statistics
struct batter {
	int batter_runs[11];
	int batter_balls[11];
	int batter_sixes[11];
	int batter_fours[11];
	float batter_strike[11];
}scores;

// Structure to store bowler statistics
struct bowler {
	int bowler_balls[5];
	int bowler_maiden[5];
	int bowler_over1[5];
	int bowler_over2[5];
	int bowler_runs[5];
	int bowler_wickets[5];
	int bowler_over_runs[5];
	int overs_bowled[5];
	float economy[5];
	int bowler_change;
}updates;

struct Client
{
	char name[90];
	int bet_runs;
	int amount;
	int status;
	int fd;
	char option;
	int clientNum;
        char choice;
}new_client;
struct Client clients[MAX_CLIENTS];


// Initialize scoreboard
void initializeScoreboard(struct score *board) {
debug_log(2 , "Initializing Scoreboard");
	// Initialize all scoreboard variables to zero
	board->runs = 0;             // Total runs scored (just for testing)
	board->target = 0;           // Target runs for the chasing team (just for testing)
	board->overs = 0;           // Total overs bowled (just for testing)
	board->wickets = 0;            // Total wickets fallen (just for testing)
	board->req_runs = 0;         // Required runs for the chasing team (just for testing)
	board->run_rate = 0;         // Current run rate (just for testing)
	board->req_run_rate = 0;     // Required run rate for the chasing team (just for testing)
	board->extra_balls = 0;        // Extra balls bowled (just for testing)
}

// Function to initialize inner match details
void initializeInnerValues(struct inner *values) {
debug_log(2 , "Initializing inner values");
	values->runs_scored = 0;        // Runs scored in the current ball
	values->overs_bowled = 0.0;     // Overs bowled in the current ball
	values->wickets_taken = 0;      // Wickets taken in the current ball
	values->total_overs = 0;        // Total overs in the match
	values->innings = 1;            // Current innings number
	values->total_balls = 0;        // Total balls bowled
	values->wide_ball = 0;          // Count of wide balls
	values->extra_ball = 0;         // Count of extra balls
	values->balls_rem = 0;          // Remaining balls in the current over
	values->current_striker = 0;    // Index of the current striker
	values->balls = 0;              // Balls bowled in the current over
}

// Function to initialize player details
void initializePlayersDetails(struct players *details) {
debug_log(2 , "Initializing player details");
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
debug_log(2 , "Initializing batter scores");
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
debug_log(2 , "Initializing bowler updates");
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


// SQLite database and statement variables
sqlite3 *db;
sqlite3_stmt *stmt;

// Function to open the SQLite database
int openDatabase() {
	// Open database file
	debug_log(2 , "Creating database file");
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
debug_log(2 , "Creating batter table in database");
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
	debug_log(2 , "Creating bowler table in database");
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
	debug_log(2 , "Creating match statistics table in database");
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
	debug_log(2 , "Inserting data in batter table");
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
	debug_log(2 , "Inserting data in bowler table");
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
debug_log(2 , "Inserting data in match statistics");
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

// Function to print debug messages based on debug level
void debug_log(int level, const char *message) {
	if (DEBUG_LEVEL <= level) {
		printf("Debug[1]: %s\n", message);
	}
}
void debuglog(int level, const char *message, size_t size) {
	if (DEBUG_LEVEL <=level) {
		printf("Debug[2]: %s %ld\n", message, size);
	}
}

// Function to update the scoreboard based on the current ball
void updateScoreboard(struct score *board, struct inner *values) {
	debug_log(2 , "Entered into the updatescoreboard function");
	// Update runs, overs, and wickets
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
	debug_log(2 , "Entered into the bowlercurrent function");
	int length;
	char extraChar;

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
	debug_log(2 , "Entered into the updatebowlerstats function");
	// If it's a regular ball (not wide or extra)
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

void batterUpdatesInnings(struct batter *scores, struct inner *values, char batter[], struct players *details) {
	debug_log(2 , "Entered into batterupdatesInnings function");
	int index = 0, current = 0;
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

// Function to display the cricket scoreboard    int team_1_batters_start_index;
void displayScoreboard(struct score *board, struct bowler *updates, struct batter *scores, struct inner *values, struct players *details) {
	// Print the scoreboard header
	//system("clear");
	debug_log(2, "Entered into display scoreboard function");
	printf("\n---------------------------- CRICKET SCOREBOARD -------------------------------\n");
	printf("%-36s vs %36s\n", details->names[details->team_1_name_index], details->names[details->team_2_name_index]);
	printf("-------------------------------------------------------------------------------\n");

	// Display batting and bowling statistics based on the innings
	if (values->innings == 1) { // First innings
		printf("     Batter          R   B   4s  6s   SR       Bowler         O   M  R  W  ECO\n");
		// Print details for top 5 batters and bowlers
		for (int i = 4; i < 9; i++) {
			int index = i - 4;
			int bowler = i + 30;
			int pos = i - 3;
			// Print batting and bowling details for each player
			printf("%2d.%-18s%-3d %-3d %-3d %-3d %-6.2f|%-16s%2d.%-2d %-2d %-3d%-2d%-6.2f\n", pos, details->names[i], scores->batter_runs[index], scores->batter_balls[index],
					scores->batter_fours[index], scores->batter_sixes[index], scores->batter_strike[index], details->names[bowler], updates->bowler_over1[index],
					updates->bowler_over2[index], updates->bowler_maiden[index], updates->bowler_runs[index], updates->bowler_wickets[index], updates->economy[index]);
		}
		// Print details for remaining batters
		for (int j = 9; j < 15; j++) {
			int index1 = j - 4;
			int pos1 = j - 3;
			printf("%2d.%-18s%-3d %-3d %-3d %-3d %-6.2f\n", pos1, details->names[j], scores->batter_runs[index1], scores->batter_balls[index1], scores->batter_fours[index1],
					scores->batter_sixes[index1], scores->batter_strike[index1]);
		}
	} else if (values->innings == 2) { // Second innings
		printf("     Batter          R   B   4s  6s  SR       Bowler          O   M  R  W  ECO\n");
		// Print details for next 5 batters and bowlers
		for (int i = 16; i < 21; i++) {
			int index = i - 16;
			int bowler = i + 12;
			int pos = i - 15;
			// Print batting and bowling details for each player
			printf("%2d.%-18s%-3d %-3d %-3d %-3d %-6.2f|%-16s%2d.%-2d %-2d %-3d%-2d%-6.2f\n", pos, details->names[i], scores->batter_runs[index], scores->batter_balls[index],
					scores->batter_fours[index], scores->batter_sixes[index], scores->batter_strike[index], details->names[bowler], updates->bowler_over1[index],
					updates->bowler_over2[index], updates->bowler_maiden[index], updates->bowler_runs[index], updates->bowler_wickets[index], updates->economy[index]);
		}
		// Print details for remaining batters
		for (int j = 21; j < 27; j++) {
			int index1 = j - 16;
			int pos1 = j - 15;
			printf("%2d.%-18s%-3d %-3d %-3d %-3d %-6.2f\n", pos1, details->names[j], scores->batter_runs[index1], scores->batter_balls[index1], scores->batter_fours[index1],
					scores->batter_sixes[index1], scores->batter_strike[index1]);
		}
	}

	// Print scoreboard footer
	printf("-------------------------------------------------------------------------------\n");
	printf("\n");
	// Display key statistics
	printf("Runs: %-18d Wickets: %-20d Overs: %d.%d\n", board->runs, board->wickets, values->balls / MAX_BALLS_PER_OVER, values->balls % MAX_BALLS_PER_OVER);
	printf("Run Rate: %-14.2f Target: %-20d  Balls Remaining: %d\n", board->run_rate, board->target, values->balls_rem);
	printf("Required Runs: %-9d Required Run Rate: %-10.2f Extra balls: %d\n", board->req_runs, board->req_run_rate, board->extra_balls);
	printf("Overs Left: %2d.%-6d\n", values->balls_rem / 6, values->balls_rem % 6);
	printf("-----------------------------------------------------------------------------\n");
	printf("\n");
	// Print umpires' names
	printf("UMPIRES: %s   |   %s   |   %s\n", details->names[details->umpire_1_index], details->names[details->umpire_2_index], details->names[details->umpire_3_index]);
	printf("\n");
	printf("-----------------------------------------------------------------------------\n");
	debug_log(2, "Leaving from display scoreboard function");
}

