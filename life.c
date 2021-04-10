#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
 
void copy(int fd1[2], int fd2[2])
{
    fd1[0] = fd2[0];
    fd1[1] = fd2[1];
}
void closeAll(int fd[2])
{
    close(fd[0]);
    close(fd[1]);
}
 
void megaFork(int n, char *val, int *row, int *lIn, int *lOut, int *rIn, int *rOut, char isRow)
{
    int pLeftIn[2], pLeftOut[2], pRightIn[2], pRightOut[2], in[2], out[2];
    int i, j, fork_res;
    pipe(pLeftIn);
    pipe(pLeftOut);
    pipe(pRightIn);
    pipe(pRightOut);
    copy(in, pLeftIn);
    copy(out, pLeftOut);
    *val = rand() % 2;
    for (i = 1; i < n && (fork_res = fork()) == 0; i++)
    {
        if (i > 1)
        {
            close(*lIn);
            close(*lOut);
        }
        close(pRightIn[0]);
        close(pRightOut[1]);
        *lIn = pRightOut[0];
        *lOut = pRightIn[1];
        if (i == n - 1)
        {
            close(in[0]);
            close(out[1]);
            *rIn = out[0];
            *rOut = in[1];
        }
        else
        {
            pipe(pRightIn);
            pipe(pRightOut);
        }
        if (isRow)
            for (j = 0; j < n; j++)
                *val = rand() % 2;
        else
            *val = rand() % 2;
    }
    if (fork_res != 0)
    {
        if (i != 0)
        {
            close(pRightIn[1]);
            close(pRightOut[0]);
            *rIn = pRightIn[0];
            *rOut = pRightOut[1];
            if (i != 1)
            {
                closeAll(in);
                closeAll(out);
            }
        }
        if (i == 1)
        {
            close(pLeftIn[1]);
            close(pLeftOut[0]);
            *lIn = pLeftIn[0];
            *lOut = pLeftOut[1];
            *rIn = pRightIn[0];
            *rOut = pRightOut[1];
        }
    }
    *row = i;
}
 
int main(int argc, char **argv)
{
    int lIn, lOut, rIn, rOut, tIn, tOut, bIn, bOut;
    int n, i, seed, row, col, sum, parentID;
    char c, c1, val;
    setbuf(stdout, NULL);
    parentID = getpid();
    sscanf(argv[1], "%d", &n);
    if (argc == 3)
        sscanf(argv[2], "%d", &seed);
    else
        seed = time(NULL);
    srand(seed);
    megaFork(n, &val, &col, &lIn, &lOut, &rIn, &rOut, 1);
    megaFork(n, &val, &row, &tIn, &tOut, &bIn, &bOut, 0);
    if (row != 1)
    {
        close(lIn);
        close(lOut);
        close(rIn);
        close(rOut);
    }
    while (1)
    {
        sum = 0;
        if (row == 1)
        {
            if (getpid() == parentID)
            {
                printf("> Press enter (q - quit): ");
                c = getchar();
            }
            else
                read(lIn, &c, 1);
            if (c != 'q')
                printf("%d", val);
            write(bOut, &c, 1);
            read(tIn, &c, 1);
            if (c != 'q')
                printf("\n");
            write(rOut, &c, 1);
            if (getpid() == parentID)
                read(lIn, &c, 1);
            if (c == 'q')
                break;
            write(lOut, &val, 1);
            write(rOut, &val, 1);
            write(tOut, &val, 1);
            write(bOut, &val, 1);
            for (i = 0; i < n - 1; i++)
            {
                read(bIn, &c, 1);
                if (i == 0 || i == n - 2)
                    sum += c;
                write(lOut, &c, 1);
                write(rOut, &c, 1);
                read(lIn, &c, 1);
                write(bOut, &c, 1);
                read(rIn, &c1, 1);
                write(bOut, &c1, 1);
                if (i == 0 || i == 1)
                    sum += c + c1;
            }
            read(lIn, &c, 1);
            write(bOut, &c, 1);
            read(rIn, &c1, 1);
            write(bOut, &c1, 1);
            sum += c + c1;
        }
        else
        {
            read(tIn, &c, 1);
            if (c != 'q')
                printf("%d", val);
            write(bOut, &c, 1);
            if (c == 'q')
                break;
            write(tOut, &val, 1);
            if (row == n)
            {
                read(bIn, &c, 1);
                sum += c;
            }
            for (i = 0; i < n - row; i++)
            {
                read(bIn, &c, 1);
                if (i == 0)
                    sum += c;
                write(tOut, &c, 1);
            }
            if (row != n)
                write(bOut, &val, 1);
            read(tIn, &c, 1);
            sum += c;
            for (i = 0; i < n; i++)
            {
                read(tIn, &c, 1);
                read(tIn, &c1, 1);
                if (i == row - 1 || i == row || i == row - 2 || (row == n && i == 0))
                    sum += c + c1;
                if (row != n)
                {
                    write(bOut, &c, 1);
                    write(bOut, &c1, 1);
                }
            }
        }
        if (val == 0 && sum == 3)
            val = 1;
        else if (sum != 2 && sum != 3)
            val = 0;
    }
    while (wait(NULL) != -1);
    return 0;
}