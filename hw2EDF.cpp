#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <string>
#include <list>
#include <sstream>
#include <stack>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <algorithm> 

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

using namespace std;
using std::cout;
using std::endl;

string writenametomemory(string name, char* ptr, int a)
{
	int n = name.size();
	string nnn;
	for (int x = 0; x < n; x++)
	{
		ptr[a+x] = name[x];
		nnn += ptr[a + x];
	}

	return nnn;
}

bool isedf(int customerindex, int *edfarray , int arraysize)
{
	
	int curredf = edfarray[customerindex];

	for (int x = 0; x < arraysize; x++)
	{
		if (x!= customerindex)
		{
			if (edfarray[x] < curredf)
				return false;
		}
	}
	//if deadline = then smaller customernumber goes first
	for (int x = 0; x < customerindex-1; x++)
	{
			if (edfarray[x] == curredf)
				return false;
	}

	return true;
}

int main() {

	int numberofrooms;
	int numberofcustomers;

	ifstream  file1("hw2input.txt");

	string str2;

	int cobegin = 0;

	getline(file1, str2);
	numberofrooms = atoi(str2.c_str());
	getline(file1, str2);
	numberofcustomers = atoi(str2.c_str());

	int customernumbers[numberofcustomers];
	int reserve[numberofcustomers];
	int cancel[numberofcustomers];
	int check[numberofcustomers];
	int pay[numberofcustomers];
	list< list<string> > transactions[numberofcustomers];

	int customernumber;
	int customerindex = 0;

	while (getline(file1, str2))
	{
		//stringstream ss2(str2);
		//string token;
		//list<string> tokens;

		if (str2.substr(0, 9) == "customer_")
		{
			str2.erase(str2.length()-1, 1);
			customernumber = atoi(str2.substr(9).c_str());

			customernumbers[customerindex] = customernumber;
			//cout << customernumber << customerindex << customernumbers[customerindex] << endl;
			getline(file1, str2);
			reserve[customerindex] = atoi(str2.substr(8).c_str());
			getline(file1, str2);
			cancel[customerindex] = atoi(str2.substr(7).c_str());
			getline(file1, str2);
			check[customerindex] = atoi(str2.substr(6).c_str());
			getline(file1, str2);
			pay[customerindex] = atoi(str2.substr(4).c_str());

			//transactions
			while (getline(file1, str2))
			{
				if (str2.substr(0, 4) == "end.")
				{
					break;
				}
				str2 += ";";

				stringstream ss2(str2);
				string token;
				list<string> tokens;

				while (ss2.peek() != ';')
				{
					if ( ss2.peek() == ' ')
					{
						ss2.ignore();
						tokens.push_back(token);
						token = "";
					}
					else
					{
							token += ss2.peek();
							ss2.ignore();
					}
				}

				if (token != "")
				{
					tokens.push_back(token);
				}

				transactions[customerindex].push_back(tokens);
			}
			//: : :
			getline(file1, str2);
			customerindex++;

			/*int n = transactions[customernumber].size();
			for (int x = 0; x < n; x++)
			{
				int g = transactions[customernumber].front().size();
				for (int y = 0; y < g; y++)
				{
					//cout << y << endl;
					cout << transactions[customernumber].front().front() << endl;
					transactions[customernumber].front().pop_front();
				}
				transactions[customernumber].pop_front();
			}*/
		}
	}
	file1.close();

	
	//shared memory of reserve
	int shmid;
	key_t key = 1238591;
	size_t size = numberofrooms *30* sizeof(int);

	shmid = shmget(key, size, 0666 | IPC_CREAT);
	if (shmid<0)
	{
		perror("shmget");
		exit(1);
	}
	int *day = (int *)shmat(shmid, 0, 0);
	if (day<0)
	{
		perror("shmat");
		exit(1);
	}

	//shared memory of paid
	int shmid2;
	key = 1238591+1;

	shmid2 = shmget(key, size, 0666 | IPC_CREAT);
	if (shmid2<0)
	{
		perror("shmget");
		exit(1);
	}
	int *paid = (int *)shmat(shmid2, 0, 0);
	if (paid<0)
	{
		perror("shmat");
		exit(1);
	}

	//shared memory of customer names
	int shmid3;
	key = 1238591 + 2;
	size = numberofrooms * 30 * sizeof(char) * 50;
	shmid3 = shmget(key, size, 0666 | IPC_CREAT);
	if (shmid3<0)
	{
		perror("shmget");
		exit(1);
	}
	char *names = (char *)shmat(shmid3, 0, 0);
	if (names<0)
	{
		perror("shmat");
		exit(1);
	}

	//shared memory of edf
	int shmid4;
	key = 1238591 + 3;
	size = numberofcustomers;
	shmid4 = shmget(key, size, 0666 | IPC_CREAT);
	if (shmid4<0)
	{
		perror("shmget");
		exit(1);
	}
	int *edf = (int *)shmat(shmid4, 0, 0);
	if (edf<0)
	{
		perror("shmat");
		exit(1);
	}

	//populate edf with big deadlines
	for (int x = 0; x < numberofcustomers;x++)
	{
		edf[x] = 2147483647;
	}

	//ipcrm -a, remove all semaphores
	//semaphore
	int semid[numberofrooms];
	key = 1238591;
	for (int x = 0; x < numberofrooms; x++)
	{
		semid[x] = semget(key, 30, 0666 | IPC_CREAT);
		key++;
		if (semid[x] <0)
		{
			perror("semget");
			exit(1);
		}
	}
	
	int pid;
	int currentcustomernunmber;

	//create customer processes
	for (int x = 0; x < numberofcustomers; x++)
	{
		pid = fork();
		currentcustomernunmber = customernumbers[x];

		if (pid == 0)
		{
			int g = transactions[x].size();

			for (int y = 0; y < g; y++)
			{

				struct timeval start, end;

				long mtime, seconds, useconds;

				gettimeofday(&start, NULL);

				//check edf to see if earliest edf
				//it will write the deadline of this transaction to the array
				//and check to see if the deadline is the earliest
				//if there is a tie for earliest deadline, which ever has the lowest customer number goes first
				// if it is it will start and change the deadline of the array to 2147483647, if not it will keep checking

				int deadline = atoi(transactions[x].front().back().c_str());
				edf[x] = deadline;

				//wait for 1 transactions from each customer to be in the edf array at the beginning
				if (y == 0)
				{
					bool allready;
					while (!allready)
					{
						allready = true;
						for (int o = 0; o < numberofcustomers; o++)
						{
							allready &= (edf[o] != 2147483647);
						}
					}
					gettimeofday(&start, NULL);
				}

				while (true)
				{
					if (isedf(x, edf, numberofcustomers))
					{
						edf[x] = 2147483647;
						break;
					}
				}


				string command = transactions[x].front().front();
				if (transactions[x].front().front()[2] != 'e')
				{
					list<int> rooms;
					bool contiguous;
					string temp;
					for (int z = 1 + command.find("("); z < command.find(")"); z++)
					{
						if (command[z] == ' ')
						{
							continue;
						}
						else
						{
							if (command[z] == ',' || command[z] == '-')
							{
								rooms.push_back(atoi(temp.c_str()) - 1);
								temp = "";
								if (command[z] == ',')
								{
									contiguous = false;
								}
								if (command[z] == '-')
								{
									contiguous = true;
								}
							}
							else
							{
								temp += command[z];
							}
						}
					}
					if (!temp.empty())
					{
						rooms.push_back(atoi(temp.c_str()) - 1);
					}

					if (contiguous)
					{
						int back = rooms.back();
						for (int z = 1 + rooms.front(); z < back; z++)
						{
							rooms.push_back(z);
						}
					}
					//int h = rooms.size();
					//for (int z = 0; z < h;z++)
					//{
					//	cout <<currentcustomernunmber<<"    "<< rooms.front() << endl;
					//	rooms.pop_front();
					//}

					//check to make sure rooms are valid
					bool notlegitroom = false;
					int u = rooms.size();
					for (int z = 0; z < u; z++)
					{
						int tempcheck = rooms.front();
						rooms.pop_front();
						rooms.push_back(tempcheck);
						if (tempcheck >= numberofrooms || tempcheck<0)
						{
							notlegitroom = true;
						}
					}
					if (notlegitroom)
					{
						cout << "Customer_" << currentcustomernunmber << " room number is not legit" << endl;
						continue;
					}

					list<int> days;
					transactions[x].front().pop_front();
					int startday = atoi(transactions[x].front().front().c_str());
					startday = startday - 1101;
					transactions[x].front().pop_front();
					int period = atoi(transactions[x].front().front().c_str());
					transactions[x].front().pop_front();
					for (int z = 0; z < period; z++)
					{
						//	cout << startday << endl;
						days.push_back(startday);
						startday++;
					}

					//wait for room and time to be available for access

					int rms = rooms.size();
					int dys = days.size();
					int nsops = 2 *(dys/2);
					list<int> rooms2;
					list<int> days2;
					struct sembuf *sops = (struct sembuf *) malloc(2 * dys * sizeof(struct sembuf));
					int ind = 0;

					if (dys>1)
					{
						for (int k = 0; k < dys / 2; k++)
						{
							int dy = days.front();
							days2.push_back(days.front());
							days.pop_front();

							sops[ind].sem_num = dy; /* We only use one track */
							sops[ind].sem_op = 0; /* wait for semaphore flag to become zero */
							sops[ind].sem_flg = SEM_UNDO; /* take off semaphore asynchronous  */

							sops[ind + 1].sem_num = dy;
							sops[ind + 1].sem_op = 1; /* increment semaphore -- take control of track */
							sops[ind + 1].sem_flg = SEM_UNDO | IPC_NOWAIT; /* take off semaphore */

							ind += 2;
						}


						for (int k = 0; k < rms; k++)
						{

							int rm = rooms.front();
							rooms2.push_back(rooms.front());
							rooms.push_back(rooms.front());
							rooms.pop_front();
							if ((semop(semid[rm], sops, nsops)) == -1)
							{
								perror("semop: semop failed");
								exit(1);
							}
						}


						ind = 0;
						nsops = 2 * (dys - (dys / 2));
						for (int k = dys / 2; k < dys; k++)
						{

							int dy = days.front();
							days2.push_back(days.front());
							days.pop_front();

							sops[ind].sem_num = dy; /* We only use one track */
							sops[ind].sem_op = 0; /* wait for semaphore flag to become zero */
							sops[ind].sem_flg = SEM_UNDO; /* take off semaphore asynchronous  */

							sops[ind + 1].sem_num = dy;
							sops[ind + 1].sem_op = 1; /* increment semaphore -- take control of track */
							sops[ind + 1].sem_flg = SEM_UNDO | IPC_NOWAIT; /* take off semaphore */

							ind += 2;
						}
						for (int k = 0; k < rms; k++)
						{
							int rm = rooms.front();
							rooms2.push_back(rooms.front());
							rooms.pop_front();
							if ((semop(semid[rm], sops, nsops)) == -1)
							{
								perror("semop: semop failed");
								exit(1);
							}
						}
					}
					else
					{
						nsops = 2;
						for (int k = 0; k < dys; k++)
						{
							int dy = days.front();
							days2.push_back(days.front());
							days.pop_front();

							sops[ind].sem_num = dy; /* We only use one track */
							sops[ind].sem_op = 0; /* wait for semaphore flag to become zero */
							sops[ind].sem_flg = SEM_UNDO; /* take off semaphore asynchronous  */

							sops[ind + 1].sem_num = dy;
							sops[ind + 1].sem_op = 1; /* increment semaphore -- take control of track */
							sops[ind + 1].sem_flg = SEM_UNDO | IPC_NOWAIT; /* take off semaphore */

							ind += 2;
						}


						for (int k = 0; k < rms; k++)
						{

							int rm = rooms.front();
							rooms2.push_back(rooms.front());
							rooms.push_back(rooms.front());
							rooms.pop_front();
							if ((semop(semid[rm], sops, nsops)) == -1)
							{
								perror("semop: semop failed");
								exit(1);
							}
						}
					}
					

					
				//	cout << "Customer_" << currentcustomernunmber << " in control" << endl;
					//edit the room and day shared memory
					//change day[roomnumber+time] to customer number
					//sem_num = day
					//sid[room number]

					//reserve
					if (command[2] == 's')
					{
						bool roomtaken = false;
						for (int z = 0; z < rms; z++)
						{
							for (int t = 0; t < dys; t++)
							{
								if (day[rooms2.front()*30 + days2.front()] != 0)
								{
									roomtaken = true;
								}
								days2.push_back(days2.front());
								days2.pop_front();
							}
							rooms2.push_back(rooms2.front());
							rooms2.pop_front();
						}

						if (!roomtaken)
						{
							for (int z = 0; z < rms; z++)
							{
								for (int t = 0; t < dys; t++)
								{
									day[rooms2.front() * 30 + days2.front()] = 1;

									string temp = writenametomemory(transactions[x].front().front(), names, rooms2.front() *50*30 + days2.front()*50);
									//	cout << rooms2.front() * 50 * 30 + days2.front() * 50 << endl;
									cout << "Customer_" << currentcustomernunmber<<", "<< transactions[x].front().front()<< " reserved room " << rooms2.front() << "+1 at day " << days2.front() << "+1101" << endl;


									//add to list


									days2.push_back(days2.front());
									days2.pop_front();
								}
								rooms2.push_back(rooms2.front());
								rooms2.pop_front();
							}
						}
						else
						{
							cout << "Customer_" << currentcustomernunmber << " room taken" << endl;

						}
					}
					//cancel
					if (command[2] == 'n')
					{
						bool notreserved = false;
						bool notsamecustomer = false;
						bool paid2 = false;
						for (int z = 0; z < rms; z++)
						{
							for (int t = 0; t < dys; t++)
							{
								if (day[rooms2.front() * 30 + days2.front()] ==0)
								{
										notreserved = true;
								}
								else
								{
									string temp;
									for (int u = 0; u < 50;u++)
									{
										if (names[rooms2.front() * 50 * 30 + days2.front() * 50 + u]=='\0')
										{
											break;
										}
										temp += names[rooms2.front() *50 *30 + days2.front()*50 + u];
									}
									if (temp!= transactions[x].front().front())
									{
										notsamecustomer = true;
									}
								}
								days2.push_back(days2.front());
								days2.pop_front();
							}
							rooms2.push_back(rooms2.front());
							rooms2.pop_front();
						}

						if (notreserved)
						{
							cout << "Customer_" << currentcustomernunmber << " cannot cancel because room not reserved" << endl;
						}
						else
						{
							if (notsamecustomer)
							{
								cout << "Customer_" << currentcustomernunmber << " cannot cancel because customer names don't match" << endl;
							}
							else
							{
								for (int z = 0; z < rms; z++)
								{
									for (int t = 0; t < dys; t++)
									{
										if (paid[rooms2.front() * 30 + days2.front()] == currentcustomernunmber)
										{
											paid2 = true;
										}
										days2.push_back(days2.front());
										days2.pop_front();
									}
									rooms2.push_back(rooms2.front());
									rooms2.pop_front();
								}

								if (paid2)
								{
									cout << "Customer_" << currentcustomernunmber << " cannot cancel because room paid by customer" << endl;
								}
								else
								{
									for (int z = 0; z < rms; z++)
									{
										for (int t = 0; t < dys; t++)
										{
											string temp;
											for (int u = 0; u < 50; u++)
											{
												names[rooms2.front() * 50 * 30 + days2.front() * 50 + u] = '\0';
												
											}

											day[rooms2.front() * 30 + days2.front()] = 0;
											cout << "Customer_" << currentcustomernunmber<<", "<< transactions[x].front().front() << " canceled " << rooms2.front() << "+1 for day " << days2.front() << "+1101" << endl;
											days2.push_back(days2.front());
											days2.pop_front();
										}
										rooms2.push_back(rooms2.front());
										rooms2.pop_front();
									}
								}
							}
						}
					}

					//pay
					if (command[2] == 'y')
					{
						bool notreserved = false;
						bool notsamecustomer = true;
						for (int z = 0; z < rms; z++)
						{
							for (int t = 0; t < dys; t++)
							{
								if (day[rooms2.front() * 30 + days2.front()] ==0)
								{
									notreserved = true;
								}
								else
								{
									string temp;
									for (int u = 0; u < 50; u++)
									{
										if (names[rooms2.front() * 50 * 30 + days2.front() * 50 + u] == '\0')
										{
											break;
										}
										temp += names[rooms2.front() * 50 * 30 + days2.front() * 50 + u];
									}
									if (temp != transactions[x].front().front())
									{
										notsamecustomer = true;
									}
								}
								days2.push_back(days2.front());
								days2.pop_front();
							}
							rooms2.push_back(rooms2.front());
							rooms2.pop_front();
						}

						if (notreserved)
						{
							cout << "Customer_" << currentcustomernunmber << " cannot pay because room not reserved" << endl;
						}
						else
						{
							if (notsamecustomer)
							{
								cout << "Customer_" << currentcustomernunmber << " cannot pay because customer names don't match" << endl;
							}
							{
								for (int z = 0; z < rms; z++)
								{
									for (int t = 0; t < dys; t++)
									{
										paid[rooms2.front() * 30 + days2.front()] = 1;

										cout << "Customer_" << currentcustomernunmber<<", "<< transactions[x].front().front() << " paid for room " << rooms2.front() << "+1 for day " << days2.front() << "+1101" << endl;
										days2.push_back(days2.front());
										days2.pop_front();
									}
									rooms2.push_back(rooms2.front());
									rooms2.pop_front();
								}
							}
						}

					}

					int delay;
					switch (command[2])
					{
					case 's':
						delay = reserve[x];
						break;
					case 'n':
						delay = cancel[x];
						break;
					case 'y':
						delay = pay[x];
						break;
					default:
						cout << "switch error" << endl;
						break;
					}
					//		sleep(delay);
					usleep(delay * 1000);

					//return control of semaphore
					nsops = dys;
					ind = 0;
					for (int k = 0; k < dys; k++)
					{
						int dy = days2.front();
						days2.pop_front();

						sops[ind].sem_num = dy;
						sops[ind].sem_op = -1; /* increment semaphore -- take control of track */
						sops[ind].sem_flg = SEM_UNDO | IPC_NOWAIT; /* take off semaphore */
						ind++;
					}



					for (int k = 0; k < rms; k++)
					{
						int rm = rooms2.front();
						rooms2.push_back(rooms2.front());
						rooms2.pop_front();

						if (semop(semid[rm], sops, nsops) == -1)
						{
							perror("semop: semop failed");
							exit(1);
						}
					}




					//cout << "Customer_"<<currentcustomernunmber << " give out control" << endl;
				}
				else
				{
					//check
					//wait for all semaphores to become available.
					int rms = numberofrooms;
					int dys = 30;
					int nsops = dys;
					struct sembuf *sops = (struct sembuf *) malloc(2 * dys * sizeof(struct sembuf));

					int ind = 0;
					for (int k = 0; k < dys/2; k++)
					{
						sops[ind].sem_num = k; /* We only use one track */
						sops[ind].sem_op = 0; /* wait for semaphore flag to become zero */
						sops[ind].sem_flg = SEM_UNDO; /* take off semaphore asynchronous  */

						sops[ind + 1].sem_num = k;
						sops[ind + 1].sem_op = 1; /* increment semaphore -- take control of track */
						sops[ind + 1].sem_flg = SEM_UNDO | IPC_NOWAIT; /* take off semaphore */

						ind += 2;
					}


					for (int k = 0; k < rms; k++)
					{

						if ((semop(semid[k], sops, nsops)) == -1)
						{
							perror("semop: semop failed");
							exit(1);
						}
					}

					ind = 0;
					for (int k = dys/2; k < dys; k++)
					{
						sops[ind].sem_num = k; /* We only use one track */
						sops[ind].sem_op = 0; /* wait for semaphore flag to become zero */
						sops[ind].sem_flg = SEM_UNDO; /* take off semaphore asynchronous  */

						sops[ind + 1].sem_num = k;
						sops[ind + 1].sem_op = 1; /* increment semaphore -- take control of track */
						sops[ind + 1].sem_flg = SEM_UNDO | IPC_NOWAIT; /* take off semaphore */

						ind += 2;
					}

					for (int k = 0; k < rms; k++)
					{

						if ((semop(semid[k], sops, nsops)) == -1)
						{
							perror("semop: semop failed");
							exit(1);
						}
					}

				//	cout << "Customer_" << currentcustomernunmber << " in control" << endl;
					transactions[x].front().pop_front();
					for (int z = 0; z < rms; z++)
					{
						for (int t = 0; t < dys; t++)
						{

							if (day[z*30+t]!=0)
							{
								string temp;
								for (int u = 0; u < 50; u++)
								{
									if (names[z * 50 * 30 + t * 50 + u] == '\0')
									{
										break;
									}
									temp += names[z * 50 * 30 + t * 50 + u];
								}
								if (temp == transactions[x].front().front())
								{
									cout <<temp<<" reserved room " <<z <<"+1 "<<" for day "<< t <<"+1101"<< endl;
								}
							}
								
						}
					}

					//delay
					usleep(check[x] * 1000);
					//return control of semaphore
					nsops = dys;
					ind = 0;
					for (int k = 0; k < dys; k++)
					{

						sops[ind].sem_num = k;
						sops[ind].sem_op = -1; /* increment semaphore -- take control of track */
						sops[ind].sem_flg = SEM_UNDO | IPC_NOWAIT; /* take off semaphore */
						ind++;
					}

					for (int k = 0; k < rms; k++)
					{
						if (semop(semid[k], sops, nsops) == -1)
						{
							perror("semop: semop failed");
						}
					}
				//	cout << "Customer_" << currentcustomernunmber << " give out control" << endl;

				}

				gettimeofday(&end, NULL);

				seconds = end.tv_sec - start.tv_sec;
				useconds = end.tv_usec - start.tv_usec;

				mtime = ((seconds)* 1000 + useconds / 1000.0) + 0.5;

				
				cout << "This trasaction took " << mtime << " milliseconds, the deadline is " << transactions[x].front().back()<< " milliseconds" << endl;

				transactions[x].pop_front();
			}

			break;
		}

		cout << "Customer_" << currentcustomernunmber << ", pid: " << pid << endl;
	}
	if (pid==0)
	{
		//deatch shared memory for children
		if (shmdt(day) == -1)
		{
			perror("shmdt");
			exit(1);
		}
		if (shmdt(paid) == -1)
		{
			perror("shmdt");
			exit(1);
		}
		if (shmdt(names) == -1)
		{
			perror("shmdt");
			exit(1);
		}
		if (shmdt(edf) == -1)
		{
			perror("shmdt");
			exit(1);
		}
	}

	if (pid!=0)
	{
		//wait for all customers to finish
		for (int x = 0; x < numberofcustomers;x++)
		{
			wait(NULL);
		}		
		cout << "REPORT" << endl;
		cout << "REPORT" << endl;
		cout << "REPORT" << endl;
		//print out report of room and date assignments 
		for (int z = 0; z < numberofrooms; z++)
		{
			for (int t = 0; t < 30; t++)
			{

				if (day[z * 30 + t] != 0)
				{
					string temp;
					for (int u = 0; u < 50; u++)
					{
						if (names[z * 50 * 30 + t * 50 + u] == '\0')
						{
							break;
						}
						temp += names[z * 50 * 30 + t * 50 + u];
					}
						cout << temp << " reserved room " << z << "+1 " << " for day " << t << "+1101" << endl;
				}

			}
		}

		//detach and destroy shared memroy
		if (shmdt(day) ==-1)
		{
			perror("shmdt");
			exit(1);
		}
		if (shmdt(paid) == -1)
		{
			perror("shmdt");
			exit(1);
		}
		if (shmdt(names) == -1)
		{
			perror("shmdt");
			exit(1);
		}
		if (shmdt(edf) == -1)
		{
			perror("shmdt");
			exit(1);
		}
		if (shmctl(shmid,IPC_RMID, 0) == -1)
		{
			perror("shmctl");
			exit(1);
		}
		if (shmctl(shmid2,IPC_RMID, 0) == -1)
		{
			perror("shmctl");
			exit(1);
		}
		if (shmctl(shmid3,IPC_RMID, 0) == -1)
		{
			perror("shmctl");
			exit(1);
		}
		if (shmctl(shmid4, IPC_RMID, 0) == -1)
		{
			perror("shmctl");
			exit(1);
		}
		//destroy semaphores
		for (int x = 0; x < numberofrooms; x++)
		{
			if (semctl(semid[x], 0, IPC_RMID, 0)==-1)
			{
				perror("semctl");
				exit(1);
			}
		}
	}

}