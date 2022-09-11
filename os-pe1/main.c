#include <stdio.h>   // for printf/scanf
#include <stdlib.h>  // for exit()
#include <unistd.h>  // for fork()
#include <time.h>   // for tiden
#include <signal.h>  
#include <sys/types.h>
#include <string.h>




//Definerer nødvendige funskjoner
int validInput(int yyyy, int mm, int dd, int s, int m, int h); //Sjekker om gyldig dato og tid

//Main-funksjonen
int main(void) {

  //Lager en struct for alarm som inneholder et nummer, pid og tid
  struct alarm{
    int nr;
    int pid;
    time_t alarmTime;
  };

  //Lager en array med datatypen alarm og som består av 10 plasser 
  struct alarm alarmList[10] = {0};

  //Initierer nødvendige variabler
  int delay, pid, i, status;
  int year, month, day, hour, min, sec;
  time_t currentTime; //Variabel for nåværende tid
  currentTime = time(NULL); //Setter næværende tid
  struct tm alarmTime; //Variabel for å legge til ny alarm
  time_t convertedTime; //Konverterer tiden 
  int numberOfAlarms = 0; //Variabel for når programmet skal avsluttes og alarmnummer
  int deleteAlarm = 0;

  //Ønsker bruker velkommen til programmet (hovedmenyen)
  printf("\nWelcome to the alarm clock! It is currently %s", ctime(&currentTime));
  printf("Please enter 's' (schedule), 'l' (list), 'c' (cancel), 'x' (exit): ");
         
  //Bruker vil kunne velge helt til de skriver "x"
  while (1){

    char choice; //Varabel for input fra bruker
    scanf(" %c", &choice); //Leser input fra bruker, for å unngå whitespace skriver vi ' %c'
    waitpid(pid, &status, WNOHANG); //Catching the zombies!

    //Sletter alarmer som har ringt ved å finne alarmer som er i fortiden
    if (deleteAlarm >= 0){  
      for (int i = 0; i < numberOfAlarms; i++){
        time_t alarmT = alarmList[i].alarmTime;
        currentTime = time(NULL); //Setter næverende tid
        delay = difftime(alarmT, currentTime); //Regner ut tid i sekunder fra nå til alarm
        //Sjekker om tidspunkt er i fortiden
        if (delay < 0){
          for (int j = i; j < numberOfAlarms; ++j){
            if (alarmList[j].nr == 10){
              break;
            }
            alarmList[j] = alarmList[j+1];
            alarmList[j].nr = j + 1;
          }
          numberOfAlarms -= 1;
          --i;
          --deleteAlarm; 
        }
      } 
    }//Avslutter slettingen av ringte alarmer

    //Start på scheduling-program
    if (choice == 's') {

      //Hindrer bruker fra å ha mer enn 10 alarmer samtidig  
      if (numberOfAlarms >= 10){
        printf("You have reached the maximum number of alarms.\nPlease enter new input\n> ");
        continue;
      }

      printf("Schedule alarm at which date and time? Format: yyyy-mm-dd hh:mm:ss\n");
      //Konverterer input fra brukeren til ønsket tidsformat
      scanf("%4d-%2d-%2d %2d:%2d:%2d", &year, &month, &day, &hour, &min ,&sec); //Input for ny alarm
      alarmTime.tm_year = year - 1900;
      alarmTime.tm_mon = month - 1;
      alarmTime.tm_mday = day;
      alarmTime.tm_hour = hour;
      alarmTime.tm_min = min;
      alarmTime.tm_sec = sec;

      //Sjekker om bruker har gitt en gyldig dato
      int valid = validInput(year, month, day, sec, min, hour);
      if (valid == 0){
        continue;
      }

      //Sette ny alarm  
      currentTime = time(NULL); //Setter næverende tid
      convertedTime = mktime(&alarmTime); // Koverterer fra struct til time_t verdi
      delay = difftime(convertedTime, currentTime); //Regner ut tid i sekunder fra nå til alarm
      //Sjekker om tidspunkt er i fortiden
      if (delay < 0){
        printf("That was %d seconds ago. Cannot schedule an alarm in the past.\nPlease enter 's' (schedule), 'l' (list), 'c' (cancel), 'x' (exit): ", delay);
        continue;
      }
      numberOfAlarms += 1;
      printf("Scheduling alarm in %d seconds.\nPlease enter new input\n> ", delay);
      pid = fork(); //Lager en ny child process ved å kalle fork()

      //child-prosess
      if (pid == 0) {                             
        sleep(delay); // venter til tiden har gått før den kan ringe alarmen
        deleteAlarm += 1;
        int cpid = getpid();                           
        system("afplay --volume 5 --time 4 alarm.mp3"); // Lyd på alarmen
        printf("ALARM\n> ");    // ringer alarmen etter tiden (delayen) har gått
        //printf("ALARM from pid %d!\a\n", cpid);    // testing right pid
        exit(0);
      } //Avslutter barneprosessen

      //Legger til alarm på ledig plass i alarmlisten
      else{
        alarmList[numberOfAlarms - 1].alarmTime = convertedTime;
        alarmList[numberOfAlarms - 1].pid = pid;
        alarmList[numberOfAlarms - 1].nr = numberOfAlarms;
      } //lukker else
    } //lukker schedule if

    //Lager en liste av alle alarmene i bruk
    else if (choice == 'l'){
      //Printer alle plassene i arrayen/listen alarmList som er i bruk
      for (i = 0; i < numberOfAlarms; ++i){ 
        printf("Alarm %d: %s", alarmList[i].nr, ctime(&alarmList[i].alarmTime)); 
      }
      printf("Number of alarms: %d\nPlease enter new input\n> ", numberOfAlarms);

    } //Avslutter list ("l")


    //Kansellering av alarm
    else if (choice == 'c'){
      int cancelAlarm;
      printf("Cancel which alarm?: ");
      scanf("%d", &cancelAlarm); //Input for kansellering av alarm     

      //Lager en løkke for å finne om alarmnummeret eksisterer i arrayen
      if (cancelAlarm < (numberOfAlarms + 1) && cancelAlarm > 0){ //Hvis alarmen finnes
        kill(alarmList[cancelAlarm-1].pid, SIGTERM);
        alarmList[cancelAlarm-1].nr = 0;

        //Lager en løkke for å fjerne alarmen
        for (int j = cancelAlarm - 1; j < numberOfAlarms; ++j){
          if (alarmList[j].nr == 10){
            break;
          }
          alarmList[j] = alarmList[j+1];
          alarmList[j].nr = j + 1;
        }
        numberOfAlarms -= 1;
        printf("Your alarm has been deleted. Please enter new input\n> ");
      }
      else{
        printf("You have no alarm with the number %d. Please enter new input\n> ", cancelAlarm);
      }
    } //Avslutter cancel ("c")

  //exit program
    else if (choice == 'x'){
      for (int i = 0; i < numberOfAlarms; i++){ //Sletter alle resterende alarmer slik at de ikke ringer etter avsluttet program
        int pid = alarmList[i].pid;
        kill(pid, SIGTERM);
        waitpid(pid, &status, WNOHANG);
      }
      printf("Goodbye!\n"); 
      break;
    } // Avslutter exit ("x")

    //Hvis bruker skriver en ugyldig input
    else{
      printf("That was not a valid input. Please try again.\n> ");
    }

  } //Avslutter while-loopen
  return 0;
} //Avslutter main


//Funskjon som sjekker om dato og tid er gyldig
int validInput(int yyyy, int mm, int dd, int s, int m, int h){
  //sjekker året 
  if(yyyy>=1900 && yyyy<=9999){
    //sjekker om gyldig måned
    if(mm>=1 && mm<=12){
      //sjekker dager
      if((dd>=1 && dd<=31) && (mm==1 || mm==3 || mm==5 || mm==7 || mm==8 || mm==10 || mm==12)){
      }
      else if((dd>=1 && dd<=30) && (mm==4 || mm==6 || mm==9 || mm==11)){
      }
      else if((dd>=1 && dd<=28) && (mm==2)){
      }
      else if(dd==29 && mm==2 && (yyyy%400==0 ||(yyyy%4==0 && yyyy%100!=0))){
      }
      else{
        printf("Day is not valid. Try again.\nPlease enter 's' (schedule), 'l' (list), 'c' (cancel), 'x' (exit): ");
        return 0;
      }
    }
    else{
      printf("Month is not valid. Try again.\nPlease enter 's' (schedule), 'l' (list), 'c' (cancel), 'x' (exit): ");
      return 0;
    }
  }
  else{
    printf("Year is not valid. Try again.\nPlease enter 's' (schedule), 'l' (list), 'c' (cancel), 'x' (exit): ");
    return 0;;
  } //Sjekker om gydlig tid
  if((h>=0&&h<24)&&(m>=0&&m<60)&&(s>=0&&s<60)){
    return 1;
  }
  else{
    printf("Time is invalid. Try again.\nPlease enter 's' (schedule), 'l' (list), 'c' (cancel), 'x' (exit): ");
    return 0;
  }
}

/*

Assumptions:
1. When alarms go off, they will be deleted.
2. When you exit the program, all the active alarms will be cancelled so that they wont
ring when you are not running the program. 

TESTCASES

Testcase 1:

1.1 What we will do:
In this test case, we will scedule an alarm using input 's'. We will decide the time of the alarm 
by writing 2022-03-15 08:15:00 in the input-field. Then we will repeat this process, but this time 
schedule the time for 2022-03-15 08:30:78, which is not a real time. This is not possible so we try again
and schedule an alarm for 2022-03-15 08:30:15, which will work. Finally we will write input 'l' which
will let us see all the sheduled alarms (here only two). 

1.2 What is going to happen / expectations:
When we run the program, we will get the welcoming message and be able to write an input. 
When we write 's', we will be asked to write a date and time. When we write 2022-03-15 08:15:00, we
will get a notification saying "Scheduling alarm in (...) seconds". When we try to schedule an alarm
by writing 2022-03-15 08:30:78, we will get a notification saying that the time is not valid and say
that we have to start over. When we schedule another alarm at 2022-03-15 08:30:15, we will get a
notification saying that it has been scheduled. When we finally write 'l', we will be able
to see all the scheduled alarms and how many there are. All the alarms will be numbered and we will see:
Alarm 1: Tue Mar 15 08:15:00 2022
Alarm 2: Tue Mar 15 08:30:15 2022
Number of alarms: 2

Testcase 2:

2.1 What we will do:
We will try and schedule 11 alarms. When we have 10 alarms, and try to schedule another one, it will
not work. So we write input 'c' so that we can cancel an alarm and make room for one more. 
We will then type 5 when asked about which alarm we would like to cancel, which will delete alarm 5. 
When we now try to schedule another alarm, we will be able to. When we type 'l', we will see
all the alarms. 

2.2 What is going to happen / expectations:
We will be able to schedule the first 10 alarms, but then when we try to schedule another one, we will
get the message "You have reached the maximum number of alarms. Please enter new input". When we then 
write 'c' we are able to cancel any alarm we want from the existing alarms. If we write 5, alarm nr 5
will be deleted. When we now write 's', we can schedule another alarm, which we will do. When we then write
'l', we will see all the alarms with numbers 1-10, where the alarm we deleted earlier should not be 
listed among them. And the last alarm we scheduled will have the number 10.

Testcase 3:

3.1 What we will do:
In this test case, we will schedule an alarm using input 's'. We will decide the time of the alarm 
by writing 2022-02-20 07:00:00 (a time in the near future) in the input-field. Then we will wait 
for the alarm to go off on the set time. Afterwards, we will write 'l' in the input field to see all
the active alarms.

3.2 What is going to happen / expectations:
When we run the program, we will get the welcoming message and be able to write an input. 
When we write 's', we will be asked to write a date and time. When we write 2022-02-15 07:00:00, we
will get a notification saying "Scheduling alarm in (...) seconds. Please enter new input". After 
the amount of seconds have went by, an alarm sound will go off, and a text line saying "Alarm".
When we write 'l' in the input field, we will see that we have no alarms since the one we 
had has already rung and is therefore deleted. 

Testcase 4:

4.1 What we will do:
In this test case, we will schedule an alarm using input 's'. We will decide the time of the alarm 
by writing 2022-02-20 07:00:00 (a time in the near future) in the input-field. Then we will cancel 
the alarm by using input 'c' and choosing the alarm we want to cancel.

4.2 What is going to happen / expectations:
When we run the program, we will get the welcoming message and be able to write an input. 
When we write 's', we will be asked to write a date and time. When we write 2022-02-15 07:00:00, we
will get a notification saying "Scheduling alarm in (...) seconds. Please enter new input". Then we will
write 'c' as an input and get "Cancel which alarm?:". Since only one alarm is schedule, this will be 
alarm 1, and we will give input '1'. After  the amount of seconds have went by, nothing will happen
because the alarm is canceled.

*/