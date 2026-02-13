Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

*Be sure to take measurements with logging disabled to ensure your logging logic is not impacting current/time measurements.*

*Please include screenshots of the profiler window detailing each current measurement captured.  See the file Instructions to add screenshots in assignment.docx in the ECEN 5823 Student Public Folder.* 

1. What is the average current per period? (Remember, once you have measured your average current, average current is average current over all time. Average current doesn’t carry with it the units of the timespan over which it was measured).
   Answer:
   <br>Screenshot:  The average current per period is 19.27μA
  <img width="1133" height="627" alt="image" src="https://github.com/user-attachments/assets/e77e52a5-13db-4955-a82a-f2e588b31a5a" />

2. What is the ave current from the time we sleep the MCU to EM3 until we power-on the 7021 in response to the LETIMER0 UF IRQ?
   Answer:
   <br>Screenshot: 2.21μA
   <img width="1467" height="541" alt="image" src="https://github.com/user-attachments/assets/390d54d2-8b0e-480c-8ccf-8d11a159d116" />
  
3. What is the ave current from the time we power-on the 7021 until we get the COMP1 IRQ indicating that the 7021's maximum time for conversion (measurement) has expired.
   Answer:108.17uA
   <br>Screenshot:  
   <img width="926" height="557" alt="image" src="https://github.com/user-attachments/assets/e1461a77-6f05-4871-8cbe-cecad2c68c8b" />

4. How long is the Si7021 Powered On for 1 temperature reading?
   Answer:
   <br>Screenshot:  96.90ms
 <img width="1136" height="645" alt="image" src="https://github.com/user-attachments/assets/c96670ee-0266-4552-b7cc-f57bd05676cc" />


5. Given the average current per period from Q1, calculate the operating time of the system for a 1000mAh battery? - ignoring battery manufacturers, efficiencies and battery derating - just a first-order calculation.
   Answer (in hours): 51894.13596 hours

6. How has the power consumption performance of your design changed since the previous assignment?
   Answer: Power consumption has significantly reduce from previous assignment in this version the avg power consumption accross total period (as showin q1 image) is **52.65nWh**
   compared to previous version (polling based) the power consumption was 479.96nWh almost a 9 fold decrease.


