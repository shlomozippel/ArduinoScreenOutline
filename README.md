ArduinoScreenOutline
====================

Light up the edges of your screen with LEDs

Requirements:
- [FastSPI_led2](http://waitingforbigo.com/2013/05/31/fastspi_led2_release_candidate/)
- PIL
- pyserial
- Python 2.7
- Arduino compatible board connected to a LED strip with a chipset supported by FastSPI (I used a ws2811 based strip)

*Windows only* due to dependency on PIL.ImageGrab.