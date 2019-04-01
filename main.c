#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <ncurses.h>
//*********DEFINES*********************************************
#define WIDTH 80
#define HEIGHT 24 
#define MAXMOVIEBUFFER 500
#define MAXNAMEBUFFER 50
#define MAXUSERS 19
//*********STRUCT PROTOTYPES***********************************
struct Movies;
//*********FUNCTION PROTOTYPES WINDOW SETUP********************
void print_menu (WINDOW *menu_win, int highlight, char* choices[], int n_choices);
int makeMenu (char* choices[], int n_choices);
char* makePopUp(int prompt);
//**********FUNCTION PROTOTYPES UI EXPLORER********************
void titleScreen (bool leaving);
bool mainMenu (bool leaving);
bool signIn ();
bool globalSettings ();
bool lookupLauncher (bool leaving);
bool catalogLauncher (bool leaving);
bool viewCatalog (int userCode);
bool displayResults (char* results, int numResults);
//**********FUNCTION PROTOTYPES GENERAL USE********************
void Upperify (char* mixedString, long mixedLength);

bool searchForMovie( char* title, char* genre, char* year);
char* justTitleSearchByTCode(char testcode[10]);
struct Movies searchByTCode (char testcode[10]);
void movieLookup ();

void inspectMovie(char*);
void inspectRecord(int userCode, int recordCode);

void exploreMovie (struct Movies target);
void addToCatalog(struct Movies film);

int selectFormat();
char* titleEntry();
char* genreSelection();
char* yearEntry();

void addUser ();
void removeUser ();
void makeCatalog (int userCode);

bool confirmDeletion(char* name);
char** allocateChoices(int num, int length);
void freeChoices (int num, char** choices);

//*********GLOBAL SETTINGS*************************************
bool dateFormat =1;
/*	
	0 for MM/DD/YYYY
 	1 for DD/MM/YYYY
*/
bool showAdult = true;
bool showAltTitle = false;
char currentUser[MAXNAMEBUFFER];
char currentUserList[MAXUSERS][MAXNAMEBUFFER];
char records[MAXUSERS][30][3][100];
int numberUsers = 1;
//*********NCURSES GLOBALS*************************************
int startx = 0;
int starty = 0;
//*********MAIN************************************************
int main() {
	/*Initializing the defulault user*/
	strcpy(currentUser,"Default");
	strcpy(currentUserList[0],"Default");
	makeCatalog(0);


	titleScreen(0);
	return 0;
}
//*********STRUCT DECLARATIONS*********************************
struct Movies {
	/*General struct for storing a movies information*/

	char tconst[10];

	char titleType[12];
	char mainTitle[MAXMOVIEBUFFER];
	char altTitle[MAXMOVIEBUFFER];

	bool isAdult;

	char startYear[5];
	char endYear[5];

	char runTime[4];

	char genres[3][15];
};
//*********WINDOW SETUP FUNCTION DECLARATIONS******************
void print_menu(WINDOW *menu_win, int highlight, char* choices[], int n_choices) {
	int x, y, i;	
	x = 2;
	y = 2;
	wborder(menu_win, '|', '|', '-', '-', '+', '+', '+', '+');
	for (i = 0; i < n_choices; i++) {
		if (highlight == i + 1) {		/* Highlight the present choice */
			wattron(menu_win, A_REVERSE); 
			mvwprintw(menu_win, y, x, "%s", choices[i]);
			wattroff(menu_win, A_REVERSE);
		} else {
			mvwprintw(menu_win, y, x, "%s", choices[i]);
		}
		++y;
	}
	wrefresh(menu_win);
}

int makeMenu(char* choices[], int n_choices) {
	WINDOW *menu_win;
	int highlight = 1;
	int choice = 0;
	int controlKey;

	initscr();
	clear();
	noecho();
	cbreak();							/* Line buffering disabled. pass on everything */
	startx = (80 - WIDTH) / 2;
	starty = (24 - HEIGHT) / 2;
		
	menu_win = newwin(HEIGHT, WIDTH, starty, startx);
	keypad(menu_win, TRUE);
	mvprintw(0, 0, "Use arrow keys to go up and down, Press enter to select a choice");
	refresh();
	print_menu(menu_win, highlight,choices, n_choices);
	while (1) {
		controlKey = wgetch(menu_win);
		switch (controlKey) {
		case KEY_UP:
			if(highlight == 1) {
				highlight = n_choices;
			} else {
				--highlight;
			}
			break;
		case KEY_DOWN:
			if(highlight == n_choices) {
				highlight = 1;
			} else { 
				++highlight;
			}
			break;
		case 10:
			choice = highlight;
			break;
		default:
			mvprintw(24, 0, "Charcter pressed is = %3d Hopefully it can be printed as '%c'", controlKey, controlKey);
			refresh();
			break;
		}
		print_menu(menu_win, highlight,choices, n_choices);
		if (choice != 0) {				/* User did a choice come out of the infinite loop */
			break;
		}
	}
	clear();
	refresh();
	endwin();
	return choice;
}

char* makePopUp(int prompt) {
	bool flag = false;
	char answer;
	char* result;
	int maxInputSize = 0;
	int row,col;
	char mesg[500];

	switch (prompt) {
	case 1:
		strcpy(mesg,"What is your name? ");
		maxInputSize = MAXNAMEBUFFER;
		break;
	case 2:
		strcpy(mesg,"What was the date you saw the movie?(MM/DD/YYYY)");
		maxInputSize = 9;
		break;
	case 3:
		strcpy(mesg,"What movie are you looking for?");
		maxInputSize = MAXMOVIEBUFFER;
		break;
	case 4:
		strcpy(mesg,"What year are you looking for?");
		maxInputSize = 3;
		break;
	case 5:
		strcpy(mesg,"What is the file name?");
		maxInputSize = MAXNAMEBUFFER;
		break;
	case 6:
		strcpy(mesg, "ENTERING A NEW USER WILL OVERWRITE ");
		strcat(mesg, currentUserList[MAXUSERS-1]);
		maxInputSize = MAXNAMEBUFFER;
		break;
	default:
		strcpy(mesg,"Now how did you get here?");
		break;
	}
	char str[maxInputSize];

	while (!flag) {
		initscr();
		echo();
		getmaxyx(stdscr,row,col);
		mvprintw(row/2,(col-strlen(mesg))/2,"%s",mesg);
		getnstr(str, maxInputSize+1);
		if(strlen(str)<70){
			mvprintw(LINES - 2, 0, "You Entered: **%s**, is this correct?(Y/N)", str);
			answer = getch();
		} else {
			mvprintw(LINES - 2, 0, "Your Entry is too long to procede, is that ok?(Y/N)", str);
			answer = getch();
		}
		getch();
		if (answer == 'Y' || answer == 'y') {
			flag = true;
			result = calloc(strlen(str)+1, sizeof(char));
			strcpy(result, str);
		}
		clear();
		refresh();
		endwin();
	}
	return result;
}
//*********UI EXPLORER FUNCTION DECLARATIONS*******************
void titleScreen(bool leaving) {
	char* choices[] = {	"Sign in",
						"Continue",
						"Exit",
	};
	int n_choices = 3;
	int currChoice;
	while (!leaving) {
		currChoice = makeMenu(choices, n_choices);
		switch (currChoice) {
		case 1:
			leaving = signIn(leaving);
			break;
		case 2:
			leaving = mainMenu(leaving);
			break;
		case 3:
			leaving = true;
			break;
		}
	}
	return;
}

bool mainMenu(bool leaving) {
	char* choices[] = {	"Movie Lookup",
						"My Reports",
						"Settings",
						"Back",
						"EXIT",
	};
	int n_choices = 5;
	int currChoice;
	while (!leaving) {
		currChoice = makeMenu(choices, n_choices);
		switch (currChoice) {
		case 1:
			leaving = lookupLauncher(leaving);
			break;
		case 2:
			leaving = catalogLauncher(leaving);
			break;
		case 3:
			leaving = globalSettings();
			break;
		case 4:
			return leaving;
		case 5:
			leaving = true;
			break;
		}
	}
	return leaving;
}

bool signIn() {
	bool control = 0;
	int currChoice;
	int n_choices = numberUsers + 5;
	char** choices;
	while (!control) {
		n_choices = numberUsers + 5;
		choices = allocateChoices(n_choices, MAXNAMEBUFFER);
		strcpy(choices[0],"New");
		strcpy(choices[1],"Delete");
		strcpy(choices[2],"Back"); 
		strcpy(choices[3],"Exit");
		strcpy(choices[4],"----------");
		for (int i = 5, j = 0; j < numberUsers; i++, j++) {
			strcpy(choices[i],currentUserList[j]);
		}
		currChoice = makeMenu(choices, n_choices);
		freeChoices(n_choices, choices);
		switch (currChoice) {
		case 1:
			addUser();
			break;
		case 2:
			removeUser();
			break;
		case 3:
			return control;
			break;
		case 4:
			control = 1;
			break;
		case 5:
			break;
		default:
			strcpy(currentUser,currentUserList[currChoice-6]);
			return control;
		}
	}
	return control;
}
bool globalSettings(){
	//Initializing settings page
	bool control = 0;
	int n_choices = 4;
	int currChoice;
	char** choices = allocateChoices(n_choices, 32);

	if (!showAdult) {
		strcpy(choices[0], "[ ] - Show Adult Films?");
	}
	else{
		strcpy(choices[0], "[X] - Show Adult Films?");
	}
	if (showAltTitle) {
		strcpy(choices[1], "[X] - Prefer Alternate Titles?");
	} else {
		strcpy(choices[1], "[ ] - Prefer Alternate Titles?");
	}
	strcpy(choices[2],"Back");
	strcpy(choices[3],"EXIT");

	//settings functionality
	while (!control) {
		currChoice = makeMenu(choices, n_choices);
		switch (currChoice) {
		case 1:
			showAdult = !showAdult;
			if (showAdult) {
				strncpy(choices[0], "[X]", 3);
			} else {
				strncpy(choices[0], "[ ]", 3);
			}
			break;
		case 2:
			showAltTitle = !showAltTitle;
			if (showAltTitle) {
				strncpy(choices[1], "[X]", 3);
			} else {
				strncpy(choices[1], "[ ]", 3);
			}
			break;
		case 3:
			freeChoices(n_choices, choices);
			return control;
			break;
		case 4:
			control = 1;
			break;
		}
	}
	freeChoices(n_choices, choices);
	return control;
}
bool lookupLauncher(bool leaving) {
	bool titleFlag = false;
	bool genreFlag = false;
	bool yearFlag = false;
	char** choices;
	char titleSearch[MAXMOVIEBUFFER];
	titleSearch[0] = '\0';
	char genreSearch[15];
	genreSearch[0] = '\0';
	char yearSearch[5];
	yearSearch[0] = '\0';
	char* temp;
	int n_choices = 7;
	int currChoice;

	while (!leaving) {
		choices = allocateChoices(n_choices, MAXMOVIEBUFFER);
		if (!titleFlag) {
			strcpy(choices[0],"Title");
		} else if(strlen(titleSearch) > 75){
			strcpy(choices[0], "Title too long to Display");
		} else {
			strcpy(choices[0], titleSearch);
		}
		if (!genreFlag) {
			strcpy(choices[1],"Genre");
		} else {
			strcpy(choices[1], genreSearch);
		} 
		if (!yearFlag) {
			strcpy(choices[2],"Year");
		} else {
			strcpy(choices[2], yearSearch);
		}
		strcpy(choices[3],"Search");
		strcpy(choices[4],"Clear");
		strcpy(choices[5],"Back");
		strcpy(choices[6],"EXIT");

		currChoice = makeMenu(choices, n_choices);
		freeChoices(n_choices, choices);
		switch (currChoice) {
		case 1:
			temp = titleEntry();
			strcpy(titleSearch,temp);
			titleFlag = true;
			free(temp);
			break;
		case 2:
			temp = genreSelection();
			if(temp != NULL){
				strcpy(genreSearch,temp);
				genreFlag = true;
			}
			free(temp);
			break;
		case 3:
			temp = yearEntry();
			strcpy(yearSearch,temp);
			yearFlag = true;
			free(temp);
			break;
		case 4:
			leaving = searchForMovie(titleSearch, genreSearch, yearSearch);
		case 5:
			titleFlag = false;
			genreFlag = false;
			yearFlag = false;
			titleSearch[0] = '\0';
			genreSearch[0] = '\0';
			yearSearch[0] = '\0';
			break;
		case 6:
			return leaving;
			break;
		case 7:
			leaving = true;
			break;
		}
	}
	return leaving;
}

bool catalogLauncher(bool leaving){
	int currChoice;
	int n_choices = numberUsers + 3;
	char** choices;
	choices = allocateChoices(n_choices, MAXNAMEBUFFER);
	strcpy(choices[0],"Back");
	strcpy(choices[1],"Exit");
	strcpy(choices[2],"----------");
	for (int i = 3, j = 0; j < numberUsers; i++, j++) {
		strcpy(choices[i],currentUserList[j]);
		strcat(choices[i], ".log.txt");
	}
	//currChoice = makeMenu(choices, n_choices);
	while(!leaving){
		currChoice = makeMenu(choices, n_choices);
		switch (currChoice) {
		case 1:
			freeChoices(n_choices, choices);
			return leaving;
			break;
		case 2:
			leaving = true;
			freeChoices(n_choices, choices);
			return leaving;
			break;
		case 3:
			break;
		default:
			viewCatalog(currChoice-4);
		}
	}
	freeChoices(n_choices, choices);
	return leaving;
}

bool displayResults(char* searchResults, int numResults){
	bool leaving= false;
	int i = 0;
	int numPages;
	printf("%i\n",numResults );
	if(numResults == 0){
		char* choices2[] = {"NO MOVIE FOUND OR BAD DATABASE",
							"Back",
							"EXIT",
		};
		int n_choices2 = 3;
		int currChoice2;
		while (!leaving) {
			currChoice2 = makeMenu(choices2, n_choices2);
			switch (currChoice2) {
			case 1:
				break;
			case 2:
				free(searchResults);
				return leaving;
			case 3:
				free(searchResults);
				leaving = true;
				return leaving;
			}
		}
	}
	char* movieSelection = strtok(searchResults, "	");
	numPages = (numResults/15) + 1;
	char pages[numPages][19][MAXMOVIEBUFFER];
	for(i = 0; i < numPages; i++){
		for( int j = 0; j<19; j++){
			if (j == 15){
				if(i == 0){
					strcpy(pages[i][j], "----------");
				} else{
					strcpy(pages[i][j], "Prev Page");
				}
			} else if (j == 16){
				if ( i == numPages-1){
					strcpy(pages[i][j], "----------");
				} else {
					strcpy(pages[i][j], "Next Page");
				}
			} else if (j == 17){
				strcpy(pages[i][j], "Back");
			} else if (j == 18){
				strcpy(pages[i][j], "EXIT");
			} else if (i*19 + j > numResults-1){
				strcpy(pages[i][j], "----------");
			} else {
				strcpy(pages[i][j], movieSelection);
				movieSelection =  strtok(NULL, "	");
			}
		}
	}
	free(searchResults);
	int n_choices = 19;
	int currChoice;
	int currPage = 0;
	char** choices;
	while(!leaving){
		choices = allocateChoices(n_choices, MAXMOVIEBUFFER);
		for( i=0; i<19;i++){
			strcpy(choices[i], pages[currPage][i]);
		}
		currChoice = makeMenu(choices, n_choices);
			switch(currChoice){
			case 16:
				if(currPage != 0){
					currPage--;
				}
				break;
			case 17:
				if(currPage + 1 != numPages){
					currPage++;
				}
				break;
			case 18:
				return leaving;
				break;
			case 19:
				leaving = true;
				return leaving;
				break;
			default:
				if(currPage*19 + currChoice<=numResults){
					inspectMovie(choices[currChoice-1]);
				}
				break;
			}
		freeChoices(n_choices, choices);
		}
	return 0;
}

void inspectMovie(char* movie){
	char* tcode = strtok(movie, " - ");
	struct Movies film = searchByTCode(tcode);
	char workingTitle[MAXMOVIEBUFFER];
	strcpy(workingTitle,"Title: ");
	if(showAltTitle){
		strcat(workingTitle, film.altTitle);
	} else {
		strcat(workingTitle, film.mainTitle);
	}
	char workingGenres[55];
	strcpy(workingGenres, "Genre(s): ");
	strcat(workingGenres, film.genres[0]);
	strcat(workingGenres, ", ");
	strcat(workingGenres, film.genres[1]);
	strcat(workingGenres, ", ");
	strcat(workingGenres, film.genres[2]);
	char workingFlag[12];
	if(film.isAdult){
		strcpy(workingFlag, "Is Adult");
	} else {
		strcpy(workingFlag, "Not Adult");
	}
	char* choices[] = {	film.tconst,
						workingTitle,
						workingFlag,
						film.startYear,
						film.runTime,
						workingGenres,
						"Back",
						"Add to Your Catalog",
	};
	int n_choices = 8;
	int currChoice;
	while (1) {
		currChoice = makeMenu(choices, n_choices);
		switch (currChoice) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			break;
		case 7:
			return;
		case 8:
			addToCatalog(film);
		}
	}
	return;
}

void inspectRecord(int userCode, int recordCode){
	char* choices[] = {	"Change Date",
						"Change Format",
						"Remove",
						"Back",
	};
	int n_choices = 4;
	int currChoice;
	char* date;
	int temp;
	int numRecords;
	while (1) {
		currChoice = makeMenu(choices, n_choices);
		switch (currChoice) {
		case 1:
			date = makePopUp(2);
			strcpy(records[userCode][recordCode][2] , date);
			break;
		case 2:
			temp =  selectFormat();
			char choices2[3][10] = {"DVD",
									"Blu-Ray",
									"Digital",
			};
			strcpy(records[userCode][recordCode][1] , choices2[temp]);
			break;
		case 3:
			numRecords = atoi(records[userCode][0][0]);
			for( int i = recordCode; i < numRecords; i++){
				for (int j =0; j < 3; j++){
					strcpy(records[userCode][i][j], records[userCode][i+1][j]);
				}
			}
			records[userCode][0][0][0] -=1;
			break;
		case 4:
			return;
		}
	}
	return;
}

void addToCatalog(struct Movies film){
	int userCode = 0;	
	while(strcmp(currentUser,currentUserList[userCode])){
		userCode++;
	}
	int numRecords = atoi(records[userCode][0][0]);
	int temp =  selectFormat();
	char choices[3][10] = {	"DVD",
							"Blu-Ray",
							"Digital",
	};
	char title[MAXMOVIEBUFFER];
	if(showAltTitle){
		strcpy(title, film.altTitle);
	} else {
		strcpy(title, film.mainTitle);
	}
	char* date = makePopUp(2);
	strcpy(records[userCode][++numRecords][0] , title);
	strcpy(records[userCode][numRecords][1] , choices[temp]);
	strcpy(records[userCode][numRecords][2] , date);
	records[userCode][0][0][0] +=1;
	return;
}
int selectFormat(){
	char* choices[] = {	"DVD",
						"Blu-Ray",
						"Digital",
	};
	int n_choices = 3;
	int currChoice;
	currChoice = makeMenu(choices, n_choices);
	return currChoice;
}


bool viewCatalog(int user){
	bool leaving = false;
	int i = 0;
	int k = 0;
	char str[100];
	int numResults = atoi(records[user][0][0]);
	if(numResults == 0){
		char* choices1[] = {"NO RECORDS",
							"BACK",
							"Exit",
	};
		int n_choices1 = 3;
		int currChoice1;
		while (!leaving) {
			currChoice1 = makeMenu(choices1, n_choices1);
			switch (currChoice1) {
			case 1:
				break;
			case 2:
				return leaving;
				break;
			case 3:
				leaving = true;
				return leaving;
				break;
			}
		}
	}
	strcpy(str, records[user][1][0]);
	strcat(str, " - ");
	strcat(str, records[user][1][1]);
	strcat(str, " - ");
	strcat(str, records[user][1][2]);
	int numPages;
	numPages = (numResults/15) + 1;
	char pages[numPages][19][MAXMOVIEBUFFER];
	for(i = 0; i < numPages; i++){
		for( int j = 0; j<19; j++){
			if (j == 15){
				if(i == 0){
					strcpy(pages[i][j], "----------");
				} else{
					strcpy(pages[i][j], "Prev Page");
				}
			} else if (j == 16){
				if ( i == numPages-1){
					strcpy(pages[i][j], "----------");
				} else {
					strcpy(pages[i][j], "Next Page");
				}
			} else if (j == 17){
				strcpy(pages[i][j], "Back");
			} else if (j == 18){
				strcpy(pages[i][j], "EXIT");
			} else if (i*15 + j > numResults-1){
				strcpy(pages[i][j], "----------");
			} else {
				strcpy(pages[i][j], str);
				strcpy(str, records[user][k][0]);
				strcat(str, " - ");
				strcat(str, records[user][k][1]);
				strcat(str, " - ");
				strcat(str, records[user][k][2]);;
			}
		}
	}
	int n_choices = 19;
	int currChoice;
	int currPage = 0;
	char** choices;
	while(!leaving){
		choices = allocateChoices(n_choices, MAXMOVIEBUFFER);
		for( i=0; i<19;i++){
			strcpy(choices[i], pages[currPage][i]);
		}
		currChoice = makeMenu(choices, n_choices);
			switch(currChoice){
			case 16:
				if(currPage != 0){
					currPage--;
				}
				break;
			case 17:
				if(currPage + 1 != numPages){
					currPage++;
				}
				break;
			case 18:
				return leaving;
				break;
			case 19:
				leaving = true;
				return leaving;
				break;
			default:
				if(currPage*19 + currChoice<=numResults){
					inspectRecord(user, currPage*15 + currChoice);
				}
				break;
			}
		freeChoices(n_choices, choices);
		}
	return 0;
}




//*************************************************************
void Upperify(char* mixedString, long mixedLength) {
	char temp[mixedLength];
	for (int i = 0; i < mixedLength; i++){
		if (mixedString[i] >= 'a' && mixedString[i] <= 'z') {
        	temp[i] = mixedString[i] - 32;
      	} else {
      		temp[i] = mixedString[i];
      	}
	}
	temp[mixedLength]='\0';
	strcpy(mixedString, temp);
	return;
}

char* justTitleSearchByTCode(char testcode[10]){
	char* target;
	char line[5000];
	FILE *fp;
	fp = fopen("data.tsv" , "r");
	if (fp == NULL) {
		perror("BAD FILE");
		target = "ERROR";
		return target;
	}
	while(fgets(line, 5000, fp)!= NULL){
		char* segment = strtok(line, "	");
		if (!strcmp(testcode, segment)) {
			segment = strtok(NULL, "	");
			segment = strtok(NULL, "	");
			if(!showAltTitle){
				strcpy(target, segment);
				fclose(fp);
				return target;	
			} else {
				segment = strtok(NULL, "	");
				strcpy(target, segment);
				fclose(fp);
				return target;
			}
		}

	}
	printf("MOVIE NOT FOUND, BAD CODE\n");
	fclose(fp);
	return target;
}


struct Movies searchByTCode(char testcode[10]) {
	struct Movies target;
	int i = 0;
	char line[5000];
	FILE *fp;
	fp = fopen("data.tsv" , "r");
	if (fp == NULL) {
		perror("BAD FILE");
		return target;
	}
	while(fgets(line, 5000, fp)!= NULL){
		char* segment = strtok(line, "	");
		if (!strcmp(testcode, segment)) {
			strcpy(target.tconst, segment);
			segment = strtok(NULL, "	");
			strcpy(target.titleType, segment);
			segment = strtok(NULL, "	");
			strcpy(target.mainTitle, segment);
			segment = strtok(NULL, "	");
			strcpy(target.altTitle, segment);
			segment = strtok(NULL, "	");
			target.isAdult = atoi(segment);
			segment = strtok(NULL, "	");
			strcpy(target.startYear, segment);
			segment = strtok(NULL, "	");
			strcpy(target.endYear, segment);
			segment = strtok(NULL, "	");
			strcpy(target.runTime, segment);
			segment = strtok(NULL, ",");
			while (segment!=NULL) {
				strcpy(target.genres[i++], segment);
				if (!strcmp(segment, "\\N")) {
					segment = NULL;
				} else {
				segment = strtok(NULL, ",");
				}
			}
			while (i<=2) {
				strcpy(target.genres[i++],"\\N");
			}
			fclose(fp);
			return target;
		}
	}
	printf("MOVIE NOT FOUND, BAD CODE\n");
	fclose(fp);
	return target;
}

char* titleEntry(){
	char* newName = makePopUp(3);
	return newName;
}

char* genreSelection(){
	bool firstPage = 1;
	char* newName = calloc(15, sizeof(char));
	char* choices1[] = {"Back",
						"Next Page",
						"Action",
						"Adventure",
						"Animation",
						"Biography",
						"Comedy",
						"Crime",
						"Documentary",
						"Drama",
						"Family",
						"Fantasy",
						"Film-Noir",
						"Game-Show",
						"History",
	};
	char* choices2[] = {"Back",
						"Prev Page",
						"Horror",
						"Music",
						"Musical",
						"Mystery",
						"News",
						"Reality-TV",
						"Romance",
						"Sci-Fi",
						"Sport",
						"Talk-Show",
						"Thriller",
						"War",
						"Western",
	};
	int n_choices = 15;
	int currChoice;
	currChoice = makeMenu(choices1, n_choices);
	while (currChoice == 2) {
		if (firstPage){
			currChoice = makeMenu(choices2,n_choices);
			firstPage = !firstPage;
		} else {
			currChoice = makeMenu(choices1, n_choices);
			firstPage = !firstPage;
		}
	}
	if(currChoice == 1){
		return NULL;
	} else {
		if(firstPage){
			strcpy(newName, choices1[currChoice-1]);
		} else {
			strcpy(newName, choices2[currChoice-1]);
		}
		return newName;
	}
	return NULL;
}

char* yearEntry(){
	char* newName = makePopUp(4);
	return newName;
}

void addUser(){
	char* newName;
	if(numberUsers == MAXUSERS){
		newName = makePopUp(6);
	} else{
	 	newName = makePopUp(1);
		numberUsers++;
	}
	strcpy(currentUserList[numberUsers-1], newName);
	free(newName);
	makeCatalog(numberUsers-1);
	return;
}

void makeCatalog(int userCode){
	strcpy(records[userCode][0][0] , "0");
	strcpy(records[userCode][0][1] , "NULL");
	strcpy(records[userCode][0][2] , "00/00/0000");
	return;
}

bool searchForMovie( char* title, char* genre, char* year) {
	int length = strlen(title);
	Upperify(title, length);
	char line[5000];
	char* searchResults = (char*)calloc(5000000,sizeof(char));
	strcpy(searchResults, "\0");
	char tconst[10];
	char tempTitle[MAXMOVIEBUFFER];
	char tempYear[5];
	int numResults = 0;
	FILE *fp;
	fp = fopen("data.tsv" , "r");
	if (fp == NULL) {
		perror("BAD FILE");
		return searchResults;
	}
	fgets(line, 5000, fp);
	while (fgets(line, 5000, fp) != NULL) {
		char* segment = strtok(line, "	");
		strcpy(tconst, segment);
		segment = strtok(NULL, "	");
		if (!strcmp(segment, "movie")) {
			segment = strtok(NULL, "	");
			strcpy(tempTitle, segment);
			Upperify(segment, strlen(segment));
			if (strstr(segment,title) != NULL) {
				segment = strtok(NULL, "	");
				segment = strtok(NULL, "	");
				if( showAdult || !strcmp(segment, "0")){
					segment = strtok(NULL, "	");
					if (year[0] == '\0' || !strcmp(year, segment)){
						strcpy(tempYear, segment);
						segment = strtok(NULL, "	");
						segment = strtok(NULL, "	");
						segment = strtok(NULL, "	");
						if(genre[0] == '\0' || strstr(segment, genre) != NULL){
							numResults++;
							strcat(searchResults, tconst);
							strcat(searchResults, " - ");
							strcat(searchResults, tempTitle);
							strcat(searchResults, " - ");
							strcat(searchResults, tempYear);
							strcat(searchResults, "	");
						}
					}
				}
			} else {
				segment = strtok(NULL, "	");
				strcpy(tempTitle, segment);
				Upperify(segment, strlen(segment));
				if (strstr(segment,title) != NULL) {
					segment = strtok(NULL, "	");
					segment = strtok(NULL, "	");
						if( showAdult || !strcmp(segment, "0")){
						segment = strtok(NULL, "	");
							if (year[0] == '\0' || !strcmp(year, segment)){
							strcpy(tempYear, segment);
							segment = strtok(NULL, "	");
							segment = strtok(NULL, "	");
							segment = strtok(NULL, "	");
							if(genre[0] == '\0' || strstr(segment, genre) != NULL){
								numResults++;
								strcat(searchResults, tconst);
								strcat(searchResults, " - ");
								strcat(searchResults, tempTitle);
								strcat(searchResults, " - ");
								strcat(searchResults, tempYear);
								strcat(searchResults, "	");
							}
						}
					}
				}
			}
		}
	}
	fclose(fp);
	printf("FOUND TITELS\n");
	return displayResults(searchResults, numResults);
}

void removeUser(){
	bool check = false;
	int currChoice;
	int n_choices = numberUsers+2;
	char** choices;
	choices = allocateChoices(n_choices, MAXNAMEBUFFER);
	strcpy(choices[0],"Back");
	strcpy(choices[1],"----------");
	for (int i = 2, j = 0; j < numberUsers; i++, j++) {
		strcpy(choices[i],currentUserList[j]);
	}
	while (!check) {
		currChoice = makeMenu(choices, n_choices);
		switch (currChoice) {
		case 1:
			freeChoices(n_choices, choices);
			return;
			break;
		case 2:
			break;
		default:
			check = confirmDeletion(currentUserList[currChoice-2]);
			if(check){
				for(int k = currChoice-2; k < numberUsers-1; k++){
					strcpy(currentUserList[k],currentUserList[k+1]);
				}
				numberUsers--;
			}
			break;
		}
	}
	freeChoices(n_choices, choices);
	return;
}

bool confirmDeletion(char* name){
	int currChoice;
	int n_choices = 3;
	char** choices = allocateChoices(n_choices, MAXNAMEBUFFER);
	strcpy(choices[0],"Are you sure you want to delete ");
	strcpy(choices[1],"YES");
	strcpy(choices[2],"NO");
	while(1){
		currChoice = makeMenu(choices, n_choices);
		switch(currChoice){
		case 1:
			break;
		case 2:
			freeChoices(n_choices, choices);
			return true;
		case 3:
			freeChoices(n_choices, choices);
			return false;
		}
	}
	freeChoices(n_choices, choices);
	return false;
}

char** allocateChoices(int num, int length){
	char** choices = calloc(num, sizeof(char*));
	for (int i = 0; i < num; i++) {
		choices[i] = calloc(length, sizeof(char));
	}
	return choices;
}

void freeChoices(int num, char**choices){
	for (int i = 0; i < num; i++) {
		free(choices[i]);
	}
	free(choices);
	return;
}