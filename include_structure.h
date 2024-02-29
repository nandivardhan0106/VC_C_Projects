#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_NAMES 100
#define MAX_NAME_LENGTH 1000
#define PORT 8080
#define MAX_CLIENTS 100
#define MAX_BALLS_PER_OVER 6
// Define constants
#define MAX_BALLS_PER_OVER 6
int DEBUG_LEVEL = 0; // Global variable to hold debug level

void debug_log(int level,  const char * message);
void debuglog(int level,  const char * message, size_t size);
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
};

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
  if (DEBUG_LEVEL >= level) {
    printf("Debug[%d]: %s\n", level, message);
  }
}
void debuglog(int level,  const char * message, size_t size) {
  if (DEBUG_LEVEL >= level) {
    printf("Debug[%d]: %s %ld\n", level, message,size);
  }
}
// Function to display the cricket scoreboard    int team_1_batters_start_index;
void displayScoreboard(struct score *board, struct bowler *updates, struct batter *scores, struct inner *values, struct players *details) {
	// Print the scoreboard header
	//system("clear");
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
}

