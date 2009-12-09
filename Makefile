default:
	@ cat README

all: mesg write writed

install: mesg write writed
	@ echo "Copying binaries to respective directories."
	@ echo "    mesg -> /usr/bin/mesg"
	@ cp mesg /usr/bin/mesg
	@ chown root:bin /usr/bin/mesg
	@ chmod 555 /usr/bin/mesg
	@ echo "    mesg.1 -> /usr/man/man1/mesg.1"
	@ cp mesg.1 /usr/man/man1/mesg.1
	@ chown root:bin /usr/man/man1/mesg.1
	@ chmod 444 /usr/man/man1/mesg.1
	@ echo "    write -> /usr/bin/write"
	@ cp write /usr/bin/write
	@ chown root:tty /usr/bin/write
	@ chmod 2555 /usr/bin/write
	@ echo "    write.1 -> /usr/man/man1/write.1"
	@ cp write.1 /usr/man/man1/write.1
	@ chown root:bin /usr/man/man1/write.1
	@ chmod 444 /usr/man/man1/write.1
	@ echo "    write.7 -> /usr/man/man7/write.7"
	@ cp write.7 /usr/man/man7/write.7
	@ chown root:bin /usr/man/man7/write.7
	@ chmod 444 /usr/man/man7/write.7
	@ echo "    writed -> /usr/sbin/in.writed"
	@ cp writed /usr/sbin/in.writed
	@ chown root:tty /usr/sbin/in.writed
	@ chmod 2555 /usr/sbin/in.writed	
	@ echo "    writed.8 -> /usr/man/man8/writed.8"
	@ cp writed.8 /usr/man/man8/writed.8
	@ chown root:bin /usr/man/man8/writed.8
	@ chmod 444 /usr/man/man8/writed.8

mesg: mesg.c
	@ echo "Now creating binary for 'mesg'."
	@ gcc -o mesg mesg.c
	@ strip mesg

write: write.c write.h util.c sock.c
	@ echo "Now creating binary for 'write'."
	@ gcc -c sock.c
	@ gcc -c util.c
	@ gcc -c write.c
	@ gcc -o write sock.o util.o write.o
	@ strip write

writed: writed.c write.h util.c sock.c
	@ echo "Now creating binary for 'writed'."
	@ gcc -o writed writed.c
	@ strip writed

clean:
	@- rm -rf mesg
	@- rm -rf write sock.o util.o write.o
	@- rm -rf writed
