Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

*Be sure to take measurements with logging disabled to ensure your logging logic is not impacting current/time measurements.*

*Please include screenshots of the profiler window detailing each current measurement captured.  See the file Instructions to add screenshots in assignment.docx in the ECEN 5823 Student Public Folder.* 

1. What is the average current per period? (Remember, once you have measured your average current, average current is average current over all time. Average current doesn’t carry with it the units of the timespan over which it was measured).
   Answer:
   <br>Screenshot:  The average current per period is 19.19μA
   <img width="1458" height="607" alt="image" src="https://github.com/user-attachments/assets/e77d11a4-c157-424d-ae19-554637fcc62d" />

   ![Avg_current_per_period](screenshots/assignment4/avg_current_per_period.jpg)  

2. What is the ave current from the time we sleep the MCU to EM3 until we power-on the 7021 in response to the LETIMER0 UF IRQ?
   Answer:
   <br>Screenshot: 2.21μA
   <img width="1467" height="541" alt="image" src="https://github.com/user-attachments/assets/390d54d2-8b0e-480c-8ccf-8d11a159d116" />

   ![Avg_current_LPM_Off](screenshots/assignment4/avg_current_lpm_off.jpg)  

4. What is the ave current from the time we power-on the 7021 until we get the COMP1 IRQ indicating that the 7021's maximum time for conversion (measurement) has expired.
   Answer:
   <br>Screenshot:  
   ![Avg_current_LPM_Off](screenshots/assignment4/avg_current_lpm_on.jpg)  

5. How long is the Si7021 Powered On for 1 temperature reading?
   Answer:
   <br>Screenshot:  104.84ms
   <img width="1231" height="560" alt="image" src="https://github.com/user-attachments/assets/1c02b2f4-6cc7-4c0e-a30e-1baebde3cd5c" />

   ![duration_lpm_on](screenshots/assignment4/avg_current_lpm_on.jpg)  

6. Given the average current per period from Q1, calculate the operating time of the system for a 1000mAh battery? - ignoring battery manufacturers, efficiencies and battery derating - just a first-order calculation.
   Answer (in hours): 52110.47421
<img width="872" height="138" alt="image" src="https://github.com/user-attachments/assets/428d9ad6-7c99-4588-9252-a0fc7a5cc804" />
7. How has the power consumption performance of your design changed since the previous assignment?
   Answer:
   


