## Lift Simulation using Process, Semaphores, and Shared Memory
### Project Overview
This project simulates the movement of lifts in a building using process, semaphores, and shared memory. The simulation involves multiple lifts and persons, each with their own process, interacting with each other to simulate the boarding and deboarding of persons at different floors.
[![Demo Video](lift_simulation)](media/lift_simulation.mp4)
### Features
* Simulates multiple lifts with their own characteristics
> + Lift
>    - ID
>    - Current Position
>    - Direction
>    - Capacity
>    - Peoples Inside Lift
>    - list of peoples waiting to deboard at different floors
* Simulates multiple persons with their own characteristics 
> + Person
>    - Current Floor
>    - Destination Floor
* Persons wait for a lift to arrive at their current floor with the right direction
* Persons board and deboard lifts at designated floors
* Animator to visualize the simulation
### Technical Details
1. Implemented using C/C++ with fork() for process creation
2. Semaphores used for synchronization and mutual exclusion
3. Shared memory used for inter-process communication
### Implementation Details
1. Each lift and person is simulated as a separate process created using fork()
2. Semaphores are used to synchronize access to shared resources, such as the lift's current position and capacity
3. Shared memory is used to share data between processes, such as the lift's characteristics and the person's destination floor
4. The animator is implemented using a separate thread or process that updates the visualization in real-time
> Detailed explaination of the explaination is commented inside code
### Getting Started
1. Clone the repository using `git clone`
2. Compile the code using `gcc lift.c -o lift_simulation`
3. Run the simulation using `./lift_simulation`
### Example Use Cases
* Simulate a building having 10 floors with 3 lifts and 20 persons
* Observe how persons board and deboard lifts at different floors
* Adjust lift and person characteristics to see how it affects the simulation
### Contributing
Contributions are welcome! If you'd like to report a bug or suggest a new feature, please open an issue on this repository.
