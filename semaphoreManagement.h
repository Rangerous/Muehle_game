#pragma once


int createSemaphore();
int wait_semaphore (int sid);
int signal_semaphore (int sid);