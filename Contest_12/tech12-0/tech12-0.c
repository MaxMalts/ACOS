#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/epoll.h>
#include <unistd.h>

int BytesRead(int N, int in[N], int epoll)
{
    size_t res = 0;

    char buf[10000] = "";
    struct epoll_event curEvents[10000];
	int nHandeled = 0;
    while (nHandeled < N) {
        int nEvents = epoll_wait(epoll, curEvents, sizeof(curEvents) / sizeof(curEvents[0]), -1);

        for (int i = 0; i < nEvents; ++i) {
			int curFile = curEvents[i].data.fd;
			int curBytesRead = read(curFile, buf, sizeof(buf));
			if (curBytesRead > 0) {
				res += curBytesRead;
				curBytesRead = read(curFile, buf, sizeof(buf));

			} else {
				nHandeled ++;
			    close(curFile);
			}
		}
    }

    return res;
}

extern size_t read_data_and_count(size_t N, int in[N])
{
    for (size_t i = 0; i < N; ++i) {
        int flags = fcntl(in[i], F_GETFL);
        if (-1 == flags) {
            return 0;
        }
        flags |= O_NONBLOCK;
        if (fcntl(in[i], F_SETFL, flags) == -1) {
            return 0;
        }
    }

    int epoll = epoll_create1(0);
    if (-1 == epoll) {
        return 0;
    }

    for (int i = 0; i < N; ++i) {
        struct epoll_event eventProps = {
            .events = EPOLLIN, .data.fd = in[i]};

        if (epoll_ctl(epoll, EPOLL_CTL_ADD, in[i], &eventProps) == -1) {
            return 0;
        }
    }

    size_t res = BytesRead(N, in, epoll);
    close(epoll);
    return res;
}