#include "main.h"


int main()
{
	pc.baud(115200);

	//Set up I2C
	i2c.frequency(400000);  // use fast (400 kHz) I2C
	pc.printf("CPU SystemCoreClock is %d Hz\r\n", SystemCoreClock);

	t.start();

	InitIMU();
	while(1)
	{

		// If intPin goes high, all data registers have new data
		//if(mpu9250.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01) {  // On interrupt, check if data ready interrupt

		wait(0.01);

		//  }

		serviceIMU();
		serviceQuat();

		// Serial print and/or display at 0.5 s rate independent of data rates
		delt_t = t.read_ms() - count;
		if (delt_t > 500)
		{

			servicePrint();



			myled= !myled;
			count = t.read_ms();

			if(count > 1<<21)
			{
				t.start(); // start the timer over again if ~30 minutes has passed
				count = 0;
				deltat= 0;
				lastUpdate = t.read_us();
			}
			sum = 0;
			sumCount = 0;
			}
		}

 }

void servicePrint()
{
    pc.printf("ax = %f", 1000*ax);
    pc.printf(" ay = %f", 1000*ay);
    pc.printf(" az = %f  mg\n\r", 1000*az);

    pc.printf("gx = %f", gx);
    pc.printf(" gy = %f", gy);
    pc.printf(" gz = %f  deg/s\n\r", gz);

    pc.printf("gx = %f", mx);
    pc.printf(" gy = %f", my);
    pc.printf(" gz = %f  mG\n\r", mz);

 //   tempCount = mpu9250.readTempData();  // Read the adc values
 //   temperature = ((float) tempCount) / 333.87f + 21.0f; // Temperature in degrees Centigrade
 //   pc.printf(" temperature = %f  C\n\r", temperature);

//    pc.printf("q0 = %f\n\r", q[0]);
//    pc.printf("q1 = %f\n\r", q[1]);
//    pc.printf("q2 = %f\n\r", q[2]);
//    pc.printf("q3 = %f\n\r", q[3]);

    calcAngles();

    pc.printf("Yaw, Pitch, Roll: %f %f %f\n\r", yaw, pitch, roll);
    pc.printf("average rate = %f\n\r", (float) sumCount/sum);
}

void InitIMU()
{

#define IMUdebug_print 1

	  uint8_t whoami = mpu9250.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);  // Read WHO_AM_I register for MPU-9250
	  pc.printf("I AM 0x%x\n\r", whoami); pc.printf("I SHOULD BE 0x71\n\r");

	  if (whoami == 0x71) // WHO_AM_I should always be 0x68
	  {
	    pc.printf("MPU9250 WHO_AM_I is 0x%x\n\r", whoami);
	    pc.printf("MPU9250 is online...\n\r");

	    sprintf(buffer, "0x%x", whoami);

	    wait(1);

	    mpu9250.resetMPU9250(); // Reset registers to default in preparation for device calibration
	    mpu9250.MPU9250SelfTest(SelfTest); // Start by performing self test and reporting values
	    pc.printf("x-axis self test: acceleration trim within : %f % of factory value\n\r", SelfTest[0]);
	    pc.printf("y-axis self test: acceleration trim within : %f % of factory value\n\r", SelfTest[1]);
	    pc.printf("z-axis self test: acceleration trim within : %f % of factory value\n\r", SelfTest[2]);
	    pc.printf("x-axis self test: gyration trim within : %f % of factory value\n\r", SelfTest[3]);
	    pc.printf("y-axis self test: gyration trim within : %f % of factory value\n\r", SelfTest[4]);
	    pc.printf("z-axis self test: gyration trim within : %f % of factory value\n\r", SelfTest[5]);
	    mpu9250.calibrateMPU9250(gyroBias, accelBias); // Calibrate gyro and accelerometers, load biases in bias registers
	    pc.printf("x gyro bias = %f\n\r", gyroBias[0]);
	    pc.printf("y gyro bias = %f\n\r", gyroBias[1]);
	    pc.printf("z gyro bias = %f\n\r", gyroBias[2]);
	    pc.printf("x accel bias = %f\n\r", accelBias[0]);
	    pc.printf("y accel bias = %f\n\r", accelBias[1]);
	    pc.printf("z accel bias = %f\n\r", accelBias[2]);
	    wait(2);
	    mpu9250.initMPU9250();
	    pc.printf("MPU9250 initialized for active data mode....\n\r"); // Initialize device for active mode read of acclerometer, gyroscope, and temperature
	    mpu9250.initAK8963(magCalibration);
	    pc.printf("AK8963 initialized for active data mode....\n\r"); // Initialize device for active mode read of magnetometer
	    pc.printf("Accelerometer full-scale range = %f  g\n\r", 2.0f*(float)(1<<Ascale));
	    pc.printf("Gyroscope full-scale range = %f  deg/s\n\r", 250.0f*(float)(1<<Gscale));
	    if(Mscale == 0) pc.printf("Magnetometer resolution = 14  bits\n\r");
	    if(Mscale == 1) pc.printf("Magnetometer resolution = 16  bits\n\r");
	    if(Mmode == 2) pc.printf("Magnetometer ODR = 8 Hz\n\r");
	    if(Mmode == 6) pc.printf("Magnetometer ODR = 100 Hz\n\r");
	    wait(1);
	   }
	   else
	   {
	    pc.printf("Could not connect to MPU9250: \n\r");
	    pc.printf("%#x \n",  whoami);

	    sprintf(buffer, "WHO_AM_I 0x%x", whoami);


	    while(1) ; // Loop forever if communication doesn't happen
	    }

	    mpu9250.getAres(); // Get accelerometer sensitivity
	    mpu9250.getGres(); // Get gyro sensitivity
	    mpu9250.getMres(); // Get magnetometer sensitivity
	    pc.printf("Accelerometer sensitivity is %f LSB/g \n\r", 1.0f/aRes);
	    pc.printf("Gyroscope sensitivity is %f LSB/deg/s \n\r", 1.0f/gRes);
	    pc.printf("Magnetometer sensitivity is %f LSB/G \n\r", 1.0f/mRes);
	    magbias[0] = +470.;  // User environmental x-axis correction in milliGauss, should be automatically calculated
	    magbias[1] = +120.;  // User environmental x-axis correction in milliGauss
	    magbias[2] = +125.;  // User environmental x-axis correction in milliGauss
}
