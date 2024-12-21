# CS-350-Emerging-Systems-Architecture
Projects from SNHU CS-350 Course.


For the reflection, I will talk about the thermostat program

Summarize the project and what problem it was solving.

In this project, we used the CC3220S Launch pad to build a working thermostat that would simulate cloud connectivity with a UART display. The thermostat would have 2 buttons, one to set the temperature up the other to set the temperature down. It would also have a heater function (LED representation) that would turn on or off if the heat was higher or lower than the set temperature. The program needed to check for button presses, adjusting the set point accordingly, using a timer callback function. The timer callback would then compare the set point to the current temperature by reading a temperature sensor using I2C configurations. Then it would display (cloud connection) the current temperature, the set temperature, if the heat was on or off, and the operational runtime of the program. Each of these tasks had different time intervals. So the problem that the project was solving was building a task scheduler for state machines with different timer interrupts. 

What did you do particularly well?

I think what I did well was implement the task scheduler. It was difficult to understand at first, but I kind of related the tasks to OOP, where each task was its own object, and each task attribute (such as its states, its periods, and its elapsed time) could be invoked within the iteration of the task schedule, which used the GCD of all tasks (greatest common divisor, the times were 200ms, 500ms, 1000ms, so every 100ms is the GCD) to fire the timer callback. Each timer callback would do an iterative check over all of the tasks, comparing the elapsed time to the task period. If the elapsed time was greater than or equal to the period, the task would first its defined state actions and transitions. This took careful planning to ensure that each task was designed with the requirements of their timers and functionality.

Where could you improve?

For some reason, my program produced no output. I could improve by further debugging the application until at least some output (board response) occurred so I could ensure that the board and program did what they were designed to do.

What tools and/or resources are you adding to your support network?

I have added a free project planning draw.io tool to my bookmarks. It was not as efficient as a paid for version (such as LucidChart), but it served the needs of creating a complex diagram for the state machine process for the project. Another resource I was able to add was the TI documentation page. Utilizing this resource has taught me how to filter through a lot of information quickly to find the documentation I need for a project.

What skills from this project will be particularly transferable to other projects and/or course work?

State machines, I think, will play an important roll in future project design. This data structure/architecture fits neatly into a program that can't use if statements or wait operations. Also, utilizing microcontrollers to functionally create usable hardware to help control other components of a system is going to be transferable where applicable.

How did you make this project maintainable, readable, and adaptable?

As always, following best practices is key to readability. Creating classes and modules is also a good way to keep code maintainable and adaptable. For example, instead of hard coding the task scheduler into the timer callback method, the task scheduler was built into its own function, and then the timer callback method would call that function. Keeping state actions and state transitions separate allows a programmer to quickly change the conditions of transitions as well as the actions of each state without having to read through both. Proper in-line commenting is important here (such as //State actions start, //State transitions start, etc.) can help a programmer quickly determine the section that they need to modify for adaptability. Proper naming conventions, proper data types (such as unsigned char or unsigned long), and proper initialization and use of global and local variables is key to maintaining a low resource application.
