# Notes

# On Intel processors

I have observed that on intel processors with separate power cores (i.e. e cores and p cores),
the application nearly doubled the frame rate when we used the arc ball to look around. On my laptop,
(which has a ryzen 7 7840u with 16 cores) this didn't happen. This is because i think the app switches to the P cores
when it detects the user is interacting with the app, for quick snappy responses.