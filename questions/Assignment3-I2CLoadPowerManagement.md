Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

*Be sure to take measurements with logging disabled to ensure your logging logic is not impacting current/time measurements.*

*Please include screenshots of the profiler window detailing each current measurement captured.  See the file: Instructions to add screenshots in assignment.docx in the ECEN 5823 Student Public Folder.* 

1. What is the average current per period? (Remember, once you have measured your average current, average current is average current over all time. Average current doesn’t carry with it the units of the timespan over which it was measured).
   Answer:174.76uA
   <br>Screenshot:  
<img width="1575" height="597" alt="image" src="https://github.com/user-attachments/assets/082da00c-7b6b-491f-95f4-791b5bf8d400" />

   
2. What is the ave current from the time we sleep the MCU to EM3 until we power-on the 7021 in response to the LETIMER0 UF IRQ? 
   Answer:10.18uA
   <br>Screenshot:  
<img width="1912" height="733" alt="image" src="https://github.com/user-attachments/assets/9d4e59fa-b9fc-4bd4-88c9-8d9308746861" />

   
3. What is the ave current from the time we power-on the 7021 until the polled implementation of timerWaitUs() returns, indicating that the 7021's maximum time for conversion (measurement) has expired.
   Answer: 5.08mA
   <br>Screenshot:  
<img width="1053" height="591" alt="image" src="https://github.com/user-attachments/assets/5b774f73-08ef-4094-8e37-189e3a904f8a" />

   
4. How long is the Si7021 Powered On for 1 temperature reading?  
   Answer: 98.5 ms
   <br>Screenshot:  
<img width="1912" height="656" alt="image" src="https://github.com/user-attachments/assets/aa4246ea-48b2-4d57-b00a-69fe72e187de" />
