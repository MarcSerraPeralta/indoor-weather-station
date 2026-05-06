# Building the weather station

## Software choices

- Check how many times the screen is able to refresh itself? (check datasheet)
- Can I have access to a clock? (I don't think so, maybe through WiFi...)
- Do I want to update the screen every 5min? It is not really necessary during the night...
- If I put a button there to display the info, I will not pay any attention to the weather station because it will be always stationary.
- I can use partial refreshes for the 12 times it refreshed in an hour (5min * 12 = 60min) and every hour do a full refresh rate (so that the contrast does not deteriorate).
This way I reduce the number of refreshes (or partial refreshes also count?).
- I will measure 5 times (per point) and take the average of those measurements to improve the accuracy.

