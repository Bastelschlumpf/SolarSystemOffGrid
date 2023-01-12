
## Status display via  E-Ink display (M5Paper)

### Monitor description

![Circuit](../images/M5PaperMarkup.png "M5Paper")

The entire solar system should have a display on which all important status messages and values are visible.  
In addition, two diagrams about the power curve should be displayed.  
By simple symbols it should be directly recognizable in which direction the power is currently flowing and  
whether a connection is switched on or off.  
If errors occur in the battery monitor module or the charge controller,  
these should be displayed with priority.
Furthermore, it should be visible if the data is no longer supplied by the IoBroker or if it is outdated.  

Here is the description of each area.

|Label |Description |
|------|------------|
|<span style="color: red">  1 </span>| Internal monitor version. |
|<span style="color: red">  2 </span>| The date and time of the last display update. |
|<span style="color: red">  3 </span>| The strength of the wifi connection with value and symbol. |
|<span style="color: red">  4 </span>| The current state of charge of the battery with value and symbol. |
|<span style="color: red">  5 </span>| All detailed information of the battery monitor (Victron BMV-712).
|                                    | - The current voltage |
|                                    | - The current charge or discharge current |
|                                    | - The deviation of the average point voltage |
|                                    | - How much charge was charged in total |
|                                    | - How much charge has been removed in total |
|                                    | - The number of charge cycles |
|                                    | - How many days since the last full charge |
|                                    | - The current charging mode |
|                                    | In case of an error, the problem is displayed here instead. |
|<span style="color: red">  6 </span>| Display of the solar panel icon with the current watt and the power gained today |
|<span style="color: red">  7 </span>|  The details of output power monitoring (Tasmota Elite) |
|                                    | - Grid voltage |
|                                    | - Current current consumption |
|<span style="color: red">  8 </span>| Statistical data of consumed energy for the last 7 days (watts and kWh) |
|<span style="color: red">  9 </span>| The battery icon with the current charge level |
|<span style="color: red"> 10 </span>| The current charging (+) or discharging (-) power. |
|<span style="color: red"> 11 </span>| Arrow direction of loading or unloading |
|<span style="color: red"> 12 </span>| The current energy going to the end users. |
|<span style="color: red"> 13 </span>| The arrows indicate whether the final consumers are supplied by the solar system. |
|<span style="color: red"> 14 </span>| The arrows indicate whether the end users draw power from the grid otherwise 'OFF'. |
|<span style="color: red"> 15 </span>| All detailed information of the charge controler (Victron MPPT 150/70). |
|                                    | The voltage of the solar panels |
|                                    | The delivered current of the solar panels |
|                                    | The current battery charging voltage |
|                                    | The current battery charging current |
|<span style="color: red"> 16 </span>| Statistical data of the charge controller in watts and kWh and the battery charge. |

### Wall mount

And here the whole thing with a wall mount made by 3D printing

   See https://www.thingiverse.com/thing:4767014  
   ![Wall mount](../images/M5Paper.png "WallMount")

### Inplate 6 Plus version

   I have also ported the M5Paper version to Inkplate 6 Plus for testing.
   ![Inkplate 6 Plus](../images/Inkplate6Plus.png "Inkplate 6 Plus")
