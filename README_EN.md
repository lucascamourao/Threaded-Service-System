# Threaded Service System (C)

This project implements a system with multiple threads in C that simulates a customer service process. The program has three main threads: **Reception**, **Service**, and **Analyst**. The system allows creating customers with different priorities and serving them within a patience time while logging the served customers' information in a file.

## Functionality

The system is divided into three main modules:

1. **Reception**: 
    - Creates customers with two possible priorities: high and low.
    - Inserts the customers into a service queue.
    - Each customer is assigned a random priority with a 50% chance of being high or low.

2. **Service**:
    - Removes customers from the queue and serves them.
    - High-priority customers have a shorter patience time (X / 2 ms), while low-priority customers have a longer patience time (X ms).
    - The service is considered successful if the customer is served within their patience time.

3. **Analyst**:
    - Reads a text file containing the PIDs of the served customers.
    - Prints the first 10 PIDs at a time, clearing the file after reading.

The system ends when all customers are served (if N is a specific value) or when the user presses the "s" key (if N is 0, i.e., infinite customers).

## Project Structure

- **main.c**: Main file with the thread logic and synchronization.
- **analyst.c**: Logic for the Analyst thread, which prints the PIDs of the served customers.
- **reception.c**: Logic for the Reception thread, which creates customers and inserts them into the queue.
- **service.c**: Logic for the Service thread, which removes customers from the queue and serves them.
- **queue.c**: Implementation of the service queue and access control with a mutex.
- **pids.txt**: File used to store the PIDs of served customers.

## How It Works

### General Flow
1. The **Reception** thread creates customers and inserts them into the service queue.
2. The **Service** thread removes customers from the queue, serves them, and logs their PIDs in a file.
3. The **Analyst** thread reads the file and prints the PIDs of served customers.

### How the Program Handles Priority
- **High priority**: Customers with patience time X / 2 ms.
- **Low priority**: Customers with patience time X ms.
- Customer priority is randomly assigned (50% chance for each type).

### Final Output
- When all customers are served, the program calculates and displays the satisfaction rate (customers served within the given time).
- The "Analyst" thread prints the first 10 PIDs of served customers each time and clears the file.

## Compilation and Execution

1. **Compiling the code**:
   To compile the program, use the following command:
   ```bash
   gcc -o service main.c analyst.c reception.c service.c queue.c -lpthread
