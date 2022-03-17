#include <unistd.h> 
#include <arpa/inet.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>

#define port 5556
/*
1. client
2. menu
3. train_func
4. user_func
5. user
6. main()        */

int client(int sock);
int menu(int sock,int type);
int user(int sock,int choice);
int train_func(int sock,int choice);
int user_func(int sock,int choice);
// main function at the end


//-------------"client" called from main starts here-----------------------
int client(int sock){
    
    printf("\n\n\t\t\tRAILWAY TICKET BOOKING SYSTEM\n\n");
    printf("\t1. Sign In\n");
    printf("\t2. Sign up\n");
    printf("\t3. Exit\n");

    printf("\tEnter your choice number: ");
    int choice,valid;
    scanf("%d",&choice);
    write(sock,&choice,sizeof(choice));

    // For sign in when choice = 1
    if(choice==1){
        printf("\tEnter your login id: ");
        int id;
        scanf("%d",&id);
        // for password
        char password[50];
        strcpy(password,getpass("\tEnter your password: "));
        write(sock,&id,sizeof(id));
        write(sock,&password,sizeof(password));
        // checking validity of id and password
        read(sock,&valid,sizeof(valid));
        int type;
        if(valid){
            printf("\tLogin successfully\n");
            read(sock,&type,sizeof(type));
            // calling menu function
            while(menu(sock,type)!=-1);
            return 1;
        }
        else{
            printf("\tIncorrect login id and password\n");
            return 1;
        }
    }
    // For sign up when choice = 2
    else if(choice==2){
        printf("\n\tEnter the number for type of account: \n");
        printf("\t1. Admin\n");
        printf("\t2. Agent\n");
        printf("\t3. Customer\n");
        printf("\tType: ");
        int type;
        scanf("%d",&type);
        char name[50],password[50],pin[6];
        printf("\tEnter your name: ");
        scanf("%s",name);
        // for password
        strcpy(password,getpass("\tEnter your password: "));
        
        // for admin account
        if(type==1){
            while(1){
                strcpy(pin,getpass("\tEnter the PIN for creating Admin account: "));
                if(strcmp(pin,"secret")!=0)
                    printf("\tInvalid PIN try again\n");
                else
                    break;
            }
        }
        write(sock,&type,sizeof(type));
        write(sock,&name,sizeof(name));
        write(sock,&password,strlen(password));
        int id;
        read(sock,&id,sizeof(id));
        printf("\tYour login id for further login is: %d\n",id);
        return 2;
    }
    else return 3;
}
//-------------client function ends here------------------------------

//-------------menu function which is called from client--------------
int menu(int sock, int type){
    int choice;
    
    // if admin type account
    if(type==1){
        printf("\n\t1. Perform operations on trains\n");
        printf("\t2. Perform operations on users\n");
        printf("\t3. Logout\n");
        printf("\tEnter choice number: ");
        scanf("%d",&choice);
        write(sock,&choice,sizeof(choice));
        // for trains
        if(choice==1){
            printf("\t1. Add train\n");
			printf("\t2. View train\n");
			printf("\t3. Modify train\n");
			printf("\t4. Delete train\n");
			printf("\t Your Choice: ");
			scanf("%d",&choice);	
			write(sock,&choice,sizeof(choice));
            return train_func(sock,choice);	
        }
        // for users
        else if(choice==2){
            printf("\t1. Add User\n");
			printf("\t2. View all users\n");
			printf("\t3. Modify user\n");
			printf("\t4. Delete user\n");
			printf("\t Your Choice: ");
			scanf("%d",&choice);
			write(sock,&choice,sizeof(choice));
			return user_func(sock,choice);
        }
        else if(choice==3) return -1;
    }

    // if type is agent or customer
    else if(type==2 || type==3){
        printf("\t1. Book Ticket\n");
		printf("\t2. View Bookings\n");
		printf("\t3. Update Booking\n");
		printf("\t4. Cancel booking\n");
		printf("\t5. Logout\n");
		printf("\tEnter your choice: ");
        scanf("%d",&choice);
        write(sock,&choice,sizeof(choice));
        // from here "user" function will handle
        return user(sock,choice); 
    }

}
//------------menu function ends here---------------------------------

//------------train function for operations on train------------------
int train_func(int sock, int choice){
    int valid=0;
    // Add train
    if(choice==1){
        char train_name[50];
        printf("\n\tEnter train name: ");
        scanf("%s",train_name);
        write(sock,&train_name,sizeof(train_name));
        read(sock,&valid,sizeof(valid));
        if(valid)
            printf("\n\tTrain added successfully\n");
        return valid;
    }
    // View train
    else if(choice==2){
        int n,tno,ts,as;
        char train_name[50];
        read(sock,&n,sizeof(n));
        printf("\t Train_No\tTrain_name\tTotal_seats\tAvailable_seats\n");
        while(n--){
            read(sock,&tno,sizeof(tno));
            read(sock,&train_name,sizeof(train_name));
            read(sock,&ts,sizeof(ts));
            read(sock,&as,sizeof(as));

            if(strcmp(train_name,"deleted")!=0)
                printf("\t%d\t\t%s\t\t%d\t\t%d\n",tno,train_name,ts,as);
        }
        return valid;
    }
    // modify train
    else if(choice==3){
        int ts,c=2,tno;
        char train_name[50];
        write(sock,&c,sizeof(c));
        train_func(sock,c);
        printf("\n\t Enter the Train_no to modify: ");
        scanf("%d",&tno);
        write(sock,&tno,sizeof(tno));

        printf("\n\t1.Train name\n");
        printf("\t2.Total seats\n");
        printf("\tEnter choice number: ");
        scanf("%d",&c);
        write(sock,&c,sizeof(c));
        
        // To modify train name
        if(c==1){
            read(sock,&train_name,sizeof(train_name));
            printf("\n\tCurrent name: %s",train_name);
            printf("\n\tNew name: ");
            scanf("%s",train_name);
            write(sock,&train_name,sizeof(train_name));
        }
        // To modify seats
        else if(c==2){
            read(sock,&ts,sizeof(ts));
            printf("\n\tCurrent value: %d",ts);
            printf("\n\tNew value: ");
            scanf("%d",&ts);
            write(sock,&ts,sizeof(ts));
        }
        read(sock,&valid,sizeof(valid));
        if(valid)
            printf("\n\tTrain data updated\n");
        return valid;
    }
    // Delete train
    else if(choice==4){
		int c=2,tno;
		write(sock,&c,sizeof(int));
		train_func(sock,c);
		
		printf("\n\t Enter the Train_no to delete: ");
		scanf("%d",&tno);
		write(sock,&tno,sizeof(tno));
		read(sock,&valid,sizeof(valid));
		if(valid)
			printf("\n\t Train deleted\n");
		return valid;
	}
}
//------------train function ends here--------------------------------

//------------user function for operations on user------------------
int user_func(int sock, int choice){
    int valid=0;
    // here choice refers to
    // 1. add user
    // 2. view users
    // 3. update user
    // 4. delete user
    if(choice==1){
        char name[50],password[50];
        printf("\n\tEnter the type of account:\n");
        printf("\t2. Agent\n");
        printf("\t3. Customer\n");
        printf("\tEnter choice number: ");
        int type;
        scanf("%d",&type);
        printf("\tUser name: ");
        scanf("%s",name);
        strcpy(password,getpass("\tPassword: "));
        write(sock,&type,sizeof(type));
        write(sock,&name,sizeof(name));
        write(sock,&password,strlen(password));
        read(sock,&valid,sizeof(valid));
        if(valid){
            int id;
            read(sock,&id,sizeof(id));
            printf("\tYour login id for further login is: %d\n",id);
        }
        return valid;
    }
    else if(choice==2){
        int n_users;
        read(sock,&n_users,sizeof(n_users));
        char name[50];
        int id,type;

        printf("\tU_id\tU_name\tU_type\n");
        while(n_users--){
            
            read(sock,&id,sizeof(id));
            read(sock,&name,sizeof(name));
            read(sock,&type,sizeof(type));
            
            if(strcmp(name,"deleted")!=0)
                printf("\t%d\t%s\t%d\n",id,name,type);

        }
        return valid;

    }

    else if(choice==3){
        int c=2;
        char name[50],password[50];
        write(sock,&c,sizeof(c));
        user_func(sock,c);
        printf("\n\tEnter u_id to modify: ");
        int uid;
        scanf("%d",&uid);
        write(sock,&uid,sizeof(uid));

        printf("\n\t1. User name\n");
        printf("\t2. Password\n");
        printf("Enter choice number: ");
        scanf("%d",&choice);
        write(sock,&choice,sizeof(choice));

        if(choice==1){
            read(sock,&name,sizeof(name));
            printf("\n\tCurrent name: %s",name);
            printf("\n\tNew name: ");
            scanf("%s",name);
            write(sock,&name,sizeof(name));
            read(sock,&valid,sizeof(valid));
        }
        else if(choice==2){
            printf("\n\tEnter current password: ");
            scanf("%s",password);
            write(sock,&password,sizeof(password));
            read(sock,&valid,sizeof(valid));
            if(valid){
                printf("\n\tEnter new password: ");
                scanf("%s",password);
            }
            else printf("\n\tIncorrect password\n");

            write(sock,&password,sizeof(password));
        }
        if(valid){
            read(sock,&valid,sizeof(valid));
            if(valid) printf("\n\tUser data updated\n");
        }
        return valid;
    }
    else if(choice==4){
        int c=2,id,valid=0;
        write(sock,&c,sizeof(c));
        user_func(sock,c);

        printf("\n\tEnter id to delete: ");
        scanf("%d",&id);
        write(sock,&id,sizeof(id));
        read(sock,&valid,sizeof(valid));
        if(valid) printf("\n\tUser deleted\n");
        return valid;
    }
}


//------------user function ends here-------------------------------

//--------------user starts here------------------------------------
int user(int sock, int choice){
    int valid =0;
    // Ticket bookings
	if(choice==1){										
		int view=2;
		write(sock,&view,sizeof(int));
		train_func(sock,view);
		printf("\n\tEnter the train number to book: ");
		int tid;
        scanf("%d",&tid);
		write(sock,&tid,sizeof(tid));
				
		printf("\n\tEnter the number of seats to book: ");
		int seats;
        scanf("%d",&seats);
		write(sock,&seats,sizeof(seats));
	
		read(sock,&valid,sizeof(valid));
		if(valid)
			printf("\n\tTickets booked successfully\n");
		else
			printf("\n\tSeats not available\n");

		return valid;
	}
	// view bookings
	else if(choice==2){
		int n;
		read(sock,&n,sizeof(n));
        int id;
        int tid,seats;
		printf("\tB_id\tT_no\tSeats\n");
		while(n--){
			read(sock,&id,sizeof(id));
			read(sock,&tid,sizeof(tid));
			read(sock,&seats,sizeof(seats));
			
			if(seats!=0)
				printf("\t%d\t%d\t%d\n",id,tid,seats);
		}

		return valid;
	}
    // update booking
	else if(choice==3){	
		int c = 2,valid;
		user(sock,c);
		printf("\n\t Enter the B_id you want to modify: ");
		int id;
        scanf("%d",&id);
		write(sock,&id,sizeof(id));

		printf("\n\t1. Increase number of seats");
        printf("\n\t2. Decrease number of seats\n");
		printf("\tEnter choice number: ");
		scanf("%d",&c);
		write(sock,&c,sizeof(c));
        int val;
		if(c==1){
			printf("\n\tNo. of tickets to increase: ");
			scanf("%d",&val);
			write(sock,&val,sizeof(val));
		}
		else if(c==2){
			printf("\n\tNo. of tickets to decrease: ");
			scanf("%d",&val);
			write(sock,&val,sizeof(val));
		}
		read(sock,&valid,sizeof(valid));
		if(valid)
			printf("\n\tBooking updated successfully.\n");
		else
			printf("\n\tNo more seats available.\n");
		return valid;
	}
	// cancel booking
	else if(choice==4){									
		int c = 2,valid;
		user(sock,c);
		printf("\n\t Enter the B_id to cancel: ");
		int id;
        scanf("%d",&id);
		write(sock,&id,sizeof(id));
		read(sock,&valid,sizeof(valid));
		if(valid)
			printf("\n\tBooking cancelled successfully.\n");
		else
			printf("\n\tCancellation failed.\n");
		return valid;
	}
    //log out
	else if(choice==5)
		return -1;    
}

 
//---------------user ends here-------------------------------------



//-------------main function starts here---------------------------------
int main(){

    int sock;
    struct sockaddr_in server;
    char *ip_server = "127.0.0.1";
    
    sock = socket(AF_INET,SOCK_STREAM,0);  // Creating socket

    if(sock==-1) printf("Socket not created");

    server.sin_addr.s_addr = inet_addr(ip_server);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if(connect(sock,(struct sockaddr*)&server,sizeof(server))<0)
        perror("Error, connection not established");

    while(client(sock)!=3);   // calling client function 
    close(sock);
    return 0;
    
}
//--------------main function ends here-----------------------------

