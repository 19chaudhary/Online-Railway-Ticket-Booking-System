#include <unistd.h> 
#include <arpa/inet.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define port 5556

/*
    1. structures declaration
    2. client_service
    3. login
    4. signup
    5. menu
    6. train_func
    7. user_func
    8. user

*/
struct train{
		int train_number;
		char train_name[50];
		int total_seats;
		int available_seats;
		};
struct user{
		int login_id;
		char password[50];
		char name[50];
		int type;
		};

struct booking{
		int booking_id;
		int type;
		int uid;
		int tid;
		int seats;
		};
void client_service(int sock);
void login(int client_sock);
void signup(int client_sock);
int menu(int client_sock,int type,int id);
void train_func(int client_sock);
void user_func(int client_sock);
int user(int client_sock,int choice,int type,int id);
//--------------------------------------------------------------------
void client_service(int client_sock){
    int choice;
    printf("\n\tClient connected\n");
    do{
        read(client_sock,&choice,sizeof(int));
        if(choice==1)
            login(client_sock);
        else if(choice==2)
            signup(client_sock);
        else if(choice==3)
            break;
    }while(1);
    close(client_sock);
    printf("\n\tClient disconnected\n");
}
//-------------------------------------------------------------------
void login(int client_sock){
    int id,type,valid=0,valid_user=0;
    char password[50];
    int user_fd = open("db/user",O_RDWR);
    struct user u;
    read(client_sock,&id,sizeof(id));
    read(client_sock,&password,sizeof(password));

    struct flock fl;
    fl.l_start=(id-1)*sizeof(struct user);
    fl.l_len = sizeof(struct user);
    fl.l_whence = SEEK_SET;
    fl.l_pid = getpid();
    fl.l_type=F_WRLCK;
    fcntl(user_fd,F_SETLKW,&fl);

    while(read(user_fd,&u,sizeof(u))){
        if(u.login_id==id){
            valid_user=1;
            if(!strcmp(u.password,password)){
                valid=1;
                type=u.type;
                break;
            }
            else{
                valid=0;
                break;
            }
        }
        else{
            valid_user=0;
            valid=0;
        }
    }
    // more than one terminal can be used by agent
    // multiple login basically
    // so unlocking has to be done
    // after verification
    // type 1 admin type 2 agent type 3 customer
    if(type!=3){
        fl.l_type = F_UNLCK;
        fcntl(user_fd,F_SETLK,fl);
        close(user_fd);
    }
    if(valid_user){
        write(client_sock,&valid,sizeof(valid));
        if(valid){
            write(client_sock,&type,sizeof(type));
            while(menu(client_sock,type,id)!=-1); // calling menu function
        }
    }
    else
        write(client_sock,&valid,sizeof(valid));

    // Customer is not allowed to login 
    // at same time from multiple terminals
    if(type==3){
        fl.l_type = F_UNLCK;
        fcntl(user_fd,F_SETLK,&fl);
        close(user_fd);
    }
}
//-------------------------------------------------------
void signup(int client_sock){
    int id=0,type;
    char name[50],password[50];
    int user_fd = open("db/user",O_RDWR);
    struct user u, temp;
    
    read(client_sock, &type, sizeof(type));
	read(client_sock, &name, sizeof(name));
	read(client_sock, &password, sizeof(password));

    int fp = lseek(user_fd, 0, SEEK_END);

	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_start = fp;
	fl.l_len = 0;
	fl.l_whence = SEEK_SET;
	fl.l_pid = getpid();

	fcntl(user_fd,F_SETLKW, &fl);

    // login id start from 1 if file empty
    // else from previous value
    if(fp==0){
        u.login_id = 1;
        strcpy(u.name,name);
        strcpy(u.password,password);
        u.type = type;
        write(user_fd,&u,sizeof(u));
        write(client_sock,&u.login_id,sizeof(u.login_id));
    }
    else{
        fp = lseek(user_fd,-1*sizeof(struct user),SEEK_END);
        read(user_fd,&u,sizeof(u));
        u.login_id++;
        strcpy(u.name, name);
		strcpy(u.password, password);
		u.type=type;
		write(user_fd, &u, sizeof(u));
		write(client_sock, &u.login_id, sizeof(u.login_id));
	}
	fl.l_type = F_UNLCK;
	fcntl(user_fd, F_SETLK, &fl);

	close(user_fd);

}
//----------------------------------------------------------
int menu(int client_sock,int type, int id){
    int choice;
    // here type signifies admin agent customer
    // if admin
    if(type==1){
        read(client_sock,&choice,sizeof(choice));
        // here choice refers to operations
        // on train and users
        // if operations on train
        if(choice==1){
            train_func(client_sock);
            return menu(client_sock,type,id);
        }
        // if operations on users
        else if(choice==2){
            user_func(client_sock);
            return menu(client_sock,type,id);
        }
        // log out
        else if(choice==3) return -1;
    }
    // if agent or customer
    else if(type==2 || type==1 || type==3){
        read(client_sock,&choice,sizeof(choice));
		int ret = user(client_sock,choice,type,id);
		if(ret!=5)
			return menu(client_sock,type,id);
		else if(ret==5)
			return -1;
    }
}

void train_func(int client_sock){
    int choice,valid=0;
    read(client_sock,&choice,sizeof(choice));
    // here choice number refers to
    // 1. add train
    // 2. view trains
    // 3. modify
    // 4. delete
    if(choice==1){
        int td=0;
        char tn[50];
        read(client_sock,&tn,sizeof(tn));
        struct train t, temp;
        struct flock fl;
        int train_fd = open("db/train",O_RDWR);

        t.train_number = td;
        strcpy(t.train_name,tn);
        t.total_seats = 15;
        t.available_seats = 15;

        int fp = lseek(train_fd,0,SEEK_END);

        fl.l_type = F_WRLCK;
		fl.l_start = fp;
		fl.l_len = 0;
		fl.l_whence = SEEK_SET;
		fl.l_pid = getpid();

		fcntl(train_fd, F_SETLKW, &fl);
        // if no train file empty
        if(fp==0){
            valid=1;
            write(train_fd,&t,sizeof(t));
            fl.l_type = F_UNLCK;
            fcntl(train_fd,F_SETLK,&fl);
            close(train_fd);
            write(client_sock,&valid,sizeof(valid));
        }
        else{
            valid=1;
            lseek(train_fd,-1*sizeof(struct train),SEEK_END);
            read(train_fd,&temp,sizeof(temp));
            t.train_number = temp.train_number+1;
            write(train_fd,&t,sizeof(t));
            write(client_sock,&valid,sizeof(valid));
        }
        fl.l_type = F_UNLCK;
		fcntl(train_fd, F_SETLK, &fl);
		close(train_fd);
    }
    else if(choice==2){
        int train_fd = open("db/train",O_RDONLY);
        struct flock fl;
        struct train t;

        fl.l_type = F_RDLCK;
		fl.l_start = 0;
		fl.l_len = 0;
		fl.l_whence = SEEK_SET;
		fl.l_pid = getpid();

        fcntl(train_fd, F_SETLK, &fl);

        int fp = lseek(train_fd, 0, SEEK_END);
		int n_trains = fp / sizeof(struct train);
		write(client_sock, &n_trains, sizeof(int));

		lseek(train_fd,0,SEEK_SET);
        
        while(fp != lseek(train_fd,0,SEEK_CUR)){
			read(train_fd,&t,sizeof(t));
			write(client_sock,&t.train_number,sizeof(int));
			write(client_sock,&t.train_name,sizeof(t.train_name));
			write(client_sock,&t.total_seats,sizeof(int));
			write(client_sock,&t.available_seats,sizeof(int));
		}
		valid = 1;
		fl.l_type = F_UNLCK;
		fcntl(train_fd, F_SETLK, &fl);
		close(train_fd);
    }
    else if(choice==3){
        int valid=0;
        train_func(client_sock);
        int train_id;
        struct flock fl;
        struct train t;
        int train_fd = open("db/train",O_RDWR);
        read(client_sock,&train_id,sizeof(train_id));

        fl.l_type = F_WRLCK;
		fl.l_start = (train_id)*sizeof(struct train);
		fl.l_len = sizeof(struct train);
		fl.l_whence = SEEK_SET;
		fl.l_pid = getpid();
		
		fcntl(train_fd, F_SETLKW, &fl);

		lseek(train_fd, 0, SEEK_SET);
		lseek(train_fd, (train_id)*sizeof(struct train), SEEK_CUR);
		read(train_fd, &t, sizeof(struct train));
		
        int choice;
		read(client_sock,&choice,sizeof(int));

        // here choice refers to 
        // 1. update train name
        // 2. update total seats

        if(choice==1){
            write(client_sock,&t.train_name,sizeof(t.train_name));
			read(client_sock,&t.train_name,sizeof(t.train_name));
			
		}
		else if(choice==2){					
			write(client_sock,&t.total_seats,sizeof(t.total_seats));
			read(client_sock,&t.total_seats,sizeof(t.total_seats));
		}
        lseek(train_fd, -1*sizeof(struct train), SEEK_CUR);
		write(train_fd, &t, sizeof(struct train));
		valid=1;
		write(client_sock,&valid,sizeof(valid));
		fl.l_type = F_UNLCK;
		fcntl(train_fd, F_SETLK, &fl);
		close(train_fd);
    }
    else if(choice==4){
        train_func(client_sock);
		int valid=0;
		int fd_train = open("db/train", O_RDWR);
		int train_id;
        struct flock fl;
		struct train t;
		read(client_sock,&train_id,sizeof(train_id));

		fl.l_type = F_WRLCK;
		fl.l_start = (train_id)*sizeof(struct train);
		fl.l_len = sizeof(struct train);
		fl.l_whence = SEEK_SET;
		fl.l_pid = getpid();
		
		fcntl(fd_train, F_SETLKW, &fl);
		
		lseek(fd_train, 0, SEEK_SET);
		lseek(fd_train, (train_id)*sizeof(struct train), SEEK_CUR);
		read(fd_train, &t, sizeof(struct train));
		strcpy(t.train_name,"deleted");
		lseek(fd_train, -1*sizeof(struct train), SEEK_CUR);
		write(fd_train, &t, sizeof(struct train));
		valid=1;
		
        write(client_sock,&valid,sizeof(valid));
		
        // Unlock
        
        fl.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &fl);
		close(fd_train);	
    }
}
void user_func(int client_sock){
    int choice;
    read(client_sock,&choice,sizeof(choice));
    int valid=0;
    // here choice refers to
    // 1. add user
    // 2. view user
    // 3. modify user
    // 4. delete user
    if(choice==1){
        char name[50],password[50];
        int type;
        read(client_sock,&type,sizeof(type));
        read(client_sock,&name,sizeof(name));
		read(client_sock,&password,sizeof(password));
        struct flock fl;
        struct user u;
        int fd = open("db/user",O_RDWR);
        int fp = lseek(fd,0,SEEK_END);

        fl.l_type = F_WRLCK;
		fl.l_start = fp;
		fl.l_len = 0;
		fl.l_whence = SEEK_SET;
		fl.l_pid = getpid();

		fcntl(fd, F_SETLKW, &fl);

        // if fp=0 means file empty currently
        if(fp==0){
            u.login_id=1;
            strcpy(u.name, name);
			strcpy(u.password, password);
			u.type=type;
			write(fd, &u, sizeof(u));
			valid = 1;
			write(client_sock,&valid,sizeof(int));
			write(client_sock, &u.login_id, sizeof(u.login_id));
		}
		else{
			fp = lseek(fd, -1 * sizeof(struct user), SEEK_END);
			read(fd, &u, sizeof(u));
			u.login_id++;
			strcpy(u.name, name);
			strcpy(u.password, password);
			u.type=type;
			write(fd, &u, sizeof(u));
			valid = 1;
			write(client_sock,&valid,sizeof(int));
			write(client_sock, &u.login_id, sizeof(u.login_id));
		}
		fl.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &fl);
		close(fd);
        }
    
    else if(choice==2){
        // just for view purpose

		int fd = open("db/user", O_RDONLY);
		struct flock fl;
		struct user u;
		fl.l_type = F_RDLCK;
		fl.l_start = 0;
		fl.l_len = 0;
		fl.l_whence = SEEK_SET;
		fl.l_pid = getpid();
		
		fcntl(fd, F_SETLKW, &fl);
		// calculating number of users
        int fp = lseek(fd, 0, SEEK_END);
		int n_users = fp / sizeof(struct user);
		n_users--;
		write(client_sock, &n_users, sizeof(int));

		lseek(fd,0,SEEK_SET);
		while(fp != lseek(fd,0,SEEK_CUR)){
			read(fd,&u,sizeof(u));
			if(u.type!=1){
				write(client_sock,&u.login_id,sizeof(int));
				write(client_sock,&u.name,sizeof(u.name));
				write(client_sock,&u.type,sizeof(int));
			}
		}
		valid = 1;
		fl.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &fl);
		close(fd);
    }
    else if(choice==3){
        // to update first display current users
        // so for view choice has to be 2
        user_func(client_sock);
        int choice,valid=0;
        char password[50];
        int id;
        struct flock fl;
        struct user u;
        int fd=open("db/user",O_RDWR);

        read(client_sock,&id,sizeof(id));
        
        fl.l_type = F_WRLCK;
		fl.l_start =  (id-1)*sizeof(struct user);
		fl.l_len = sizeof(struct user);
		fl.l_whence = SEEK_SET;
		fl.l_pid = getpid();
		
		fcntl(fd, F_SETLKW, &fl);

		lseek(fd, 0, SEEK_SET);
		lseek(fd, (id-1)*sizeof(struct user), SEEK_CUR);
		read(fd, &u, sizeof(struct user));
		
		read(client_sock,&choice,sizeof(int));
        // here choice refers to
        // 1. update name
        // 2. update password
        if(choice==1){
            write(client_sock,&u.name,sizeof(u.name));
			read(client_sock,&u.name,sizeof(u.name));
			valid=1;
			write(client_sock,&valid,sizeof(valid));		
		}
		else if(choice==2){				
			read(client_sock,&password,sizeof(password));
			if(!strcmp(u.password,password))
				valid = 1;
			write(client_sock,&valid,sizeof(valid));
			read(client_sock,&u.password,sizeof(u.password));
		}
        lseek(fd, -1*sizeof(struct user), SEEK_CUR);
		write(fd, &u, sizeof(struct user));
		if(valid)
			write(client_sock,&valid,sizeof(valid));
		fl.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &fl);
		close(fd);
    }
    else if(choice==4){
        // for delete
        user_func(client_sock);
        int fd=open("db/user",O_RDWR);
        int id,valid=0;
        struct user u;

        struct flock fl;
        read(client_sock,&id,sizeof(id));

        fl.l_type = F_WRLCK;
		fl.l_start =  (id-1)*sizeof(struct user);
		fl.l_len = sizeof(struct user);
		fl.l_whence = SEEK_SET;
		fl.l_pid = getpid();
		
		fcntl(fd, F_SETLKW, &fl);
		
		lseek(fd, 0, SEEK_SET);
		lseek(fd, (id-1)*sizeof(struct user), SEEK_CUR);
		read(fd, &u, sizeof(struct user));
		strcpy(u.name,"deleted");
		strcpy(u.password,"");
		lseek(fd, -1*sizeof(struct user), SEEK_CUR);
		write(fd, &u, sizeof(struct user));
		valid=1;
		write(client_sock,&valid,sizeof(valid));
		fl.l_type = F_UNLCK;
		fcntl(fd, F_SETLK, &fl);
		close(fd);	
    }
}
int user(int client_sock,int choice, int type,int id){
    int valid=0;
    // here choice refers to
    // 1. book ticket
    // 2. view booking
    // 3. update
    // 4. cancel booking
    // 5. logout

    if(choice==1){
        train_func(client_sock);
        struct flock flt;
        struct flock flb;
        struct train t;
        struct booking b;
        int train_fd = open("db/train",O_RDWR);
        int book_fd = open("db/booking",O_RDWR);
        int tid,seats;

        read(client_sock,&tid,sizeof(tid));

        flt.l_type = F_WRLCK;
		flt.l_start = tid*sizeof(struct train);
		flt.l_len = sizeof(struct train);
		flt.l_whence = SEEK_SET;
		flt.l_pid = getpid();
		
		flb.l_type = F_WRLCK;
		flb.l_start = 0;
		flb.l_len = 0;
		flb.l_whence = SEEK_END;
		flb.l_pid = getpid();
		
		fcntl(train_fd, F_SETLKW, &flt);
		lseek(train_fd,tid*sizeof(struct train),SEEK_SET);
		
		read(train_fd,&t,sizeof(t));
		read(client_sock,&seats,sizeof(seats));

        if(t.train_number==tid){
            if(t.available_seats>=seats){
				valid = 1;
				t.available_seats -= seats;
				fcntl(book_fd, F_SETLKW, &flb);
				int fp = lseek(book_fd, 0, SEEK_END);
				
				if(fp > 0){
					lseek(book_fd, -1*sizeof(struct booking), SEEK_CUR);
					read(book_fd, &b, sizeof(struct booking));
					b.booking_id++;
				}
				else 
					b.booking_id = 0;

				b.type = type;
				b.uid = id;
				b.tid = tid;
				b.seats = seats;
				write(book_fd, &b, sizeof(struct booking));
				flb.l_type = F_UNLCK;
				fcntl(book_fd, F_SETLK, &flb);
			 	close(book_fd);
			}
		
		lseek(train_fd, -1*sizeof(struct train), SEEK_CUR);
		write(train_fd, &t, sizeof(t));
		}

		flt.l_type = F_UNLCK;
		fcntl(train_fd, F_SETLK, &flt);
		close(train_fd);
		write(client_sock,&valid,sizeof(valid));
		return valid;		
    }
    else if(choice==2){
        struct flock fl;
		struct booking b;
		int fd_book = open("db/booking", O_RDONLY);
		int no_of_bookings = 0;
	
		fl.l_type = F_RDLCK;
		fl.l_start = 0;
		fl.l_len = 0;
		fl.l_whence = SEEK_SET;
		fl.l_pid = getpid();
		
		fcntl(fd_book, F_SETLKW, &fl);
	
		while(read(fd_book,&b,sizeof(b))){
			if (b.uid==id)
				no_of_bookings++;
		}

		write(client_sock, &no_of_bookings, sizeof(int));
		lseek(fd_book,0,SEEK_SET);

		while(read(fd_book,&b,sizeof(b))){
			if(b.uid==id){
				write(client_sock,&b.booking_id,sizeof(int));
				write(client_sock,&b.tid,sizeof(int));
				write(client_sock,&b.seats,sizeof(int));
			}
		}
		fl.l_type = F_UNLCK;
		fcntl(fd_book, F_SETLK, &fl);
		close(fd_book);
		return valid;
    }
    else if(choice==3){
        int choice=2;
        int bid;
        int val;
		user(client_sock,choice,type,id);
		struct booking b;
		struct train t;
		struct flock flt;
		struct flock flb;
		int fd_book = open("db/booking", O_RDWR);
		int fd_train = open("db/train", O_RDWR);
		read(client_sock,&bid,sizeof(bid));

		flb.l_type = F_WRLCK;
		flb.l_start = bid*sizeof(struct booking);
		flb.l_len = sizeof(struct booking);
		flb.l_whence = SEEK_SET;
		flb.l_pid = getpid();
		
		fcntl(fd_book, F_SETLKW, &flb);
		lseek(fd_book,bid*sizeof(struct booking),SEEK_SET);
		read(fd_book,&b,sizeof(b));
		lseek(fd_book,-1*sizeof(struct booking),SEEK_CUR);
		
		flt.l_type = F_WRLCK;
		flt.l_start = (b.tid)*sizeof(struct train);
		flt.l_len = sizeof(struct train);
		flt.l_whence = SEEK_SET;
		flt.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &flt);
		lseek(fd_train,(b.tid)*sizeof(struct train),SEEK_SET);
		read(fd_train,&t,sizeof(t));
		lseek(fd_train,-1*sizeof(struct train),SEEK_CUR);

		read(client_sock,&choice,sizeof(choice));
        
        // here choice refers to
        // 1. increase booking seats
        // 2. decrease booking seats

		if(choice==1){							
			read(client_sock,&val,sizeof(val));
			if(t.available_seats>=val){
				valid=1;
				t.available_seats -= val;
				b.seats += val;
			}
		}
		else if(choice==2){						
			valid=1;
			read(client_sock,&val,sizeof(val));
			t.available_seats += val;
			b.seats -= val;	
		}
		
		write(fd_train,&t,sizeof(t));
		flt.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &flt);
		close(fd_train);
		
		write(fd_book,&b,sizeof(b));
		flb.l_type = F_UNLCK;
		fcntl(fd_book, F_SETLK, &flb);
		close(fd_book);
		
		write(client_sock,&valid,sizeof(valid));
		return valid;

    }
    else if(choice==4){	
		int choice = 2;
        
		user(client_sock,choice,type,id);
		struct booking b;
		struct train t;
		struct flock flb;
		struct flock flt;
		int bid;
        int fd_book = open("db/booking", O_RDWR);
		int fd_train = open("db/train", O_RDWR);
		read(client_sock,&bid,sizeof(bid));

		flb.l_type = F_WRLCK;
		flb.l_start = bid*sizeof(struct booking);
		flb.l_len = sizeof(struct booking);
		flb.l_whence = SEEK_SET;
		flb.l_pid = getpid();
		
		fcntl(fd_book, F_SETLKW, &flb);
		lseek(fd_book,bid*sizeof(struct booking),SEEK_SET);
		read(fd_book,&b,sizeof(b));
		lseek(fd_book,-1*sizeof(struct booking),SEEK_CUR);
		
		flt.l_type = F_WRLCK;
		flt.l_start = (b.tid)*sizeof(struct train);
		flt.l_len = sizeof(struct train);
		flt.l_whence = SEEK_SET;
		flt.l_pid = getpid();

		fcntl(fd_train, F_SETLKW, &flt);
		lseek(fd_train,(b.tid)*sizeof(struct train),SEEK_SET);
		read(fd_train,&t,sizeof(t));
		lseek(fd_train,-1*sizeof(struct train),SEEK_CUR);

		t.available_seats += b.seats;
		b.seats = 0;
		valid = 1;

		write(fd_train,&t,sizeof(t));
		flt.l_type = F_UNLCK;
		fcntl(fd_train, F_SETLK, &flt);
		close(fd_train);
		
		write(fd_book,&b,sizeof(b));
		flb.l_type = F_UNLCK;
		fcntl(fd_book, F_SETLK, &flb);
		close(fd_book);
		
		write(client_sock,&valid,sizeof(valid));
		return valid;
		
	}
	else if(choice==5)										
		return 5;

}
int main(){

    int sd;
    struct sockaddr_in client, server;
    char buf[80];
	int c,client_sock;
    sd = socket(AF_INET,SOCK_STREAM,0);
    if(sd==-1) printf("Socket not created");
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if(bind(sd,(struct sockaddr*)&server,sizeof(server))<0)
        perror("Error bind failed");

    listen(sd,3);
    
    c = sizeof(struct sockaddr_in);
    while(1){
        client_sock = accept(sd,(struct sockaddr*)&client,(socklen_t*)&c);

        if(!fork()){
            close(sd);
            client_service(client_sock);
            exit(0);
        }
        else
            close(client_sock);
    }
    return 0;
}
