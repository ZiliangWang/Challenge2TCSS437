#pragma config(Sensor, S1,     colorLeft,     sensorEV3_Color)
#pragma config(Sensor, S3,     colorRight,    sensorEV3_Color)
#pragma config(Sensor, S2,     sonar,         sensorEV3_Ultrasonic)
#pragma config(Motor,  motorA, leftMotor,     tmotorEV3_Large, PIDControl, encoder)
#pragma config(Motor,  motorD, rightMotor,    tmotorEV3_Large, PIDControl, encoder)

#define COLOR_SENSOR_AVERAGE_BUFFER_LEN 10
#define SONAR_SENSOR_AVERAGE_BUFFER_LEN 10

float avgColorLeft = 0,
			avgColorRight = 0,
      avgDist = 0;
//this is making robot to do a random left turn.
void turnLeft() {
	long turnTime = 450;
	setMotorSpeed(leftMotor, -55);
	setMotorSpeed(rightMotor, 55);
	wait1Msec(turnTime);
}

//this is making robot to do a random left turn.
void turnRight() {
	long turnTime = 450;
	setMotorSpeed(leftMotor, 55);
	setMotorSpeed(rightMotor, -55);
	wait1Msec(turnTime);
}

//this will give robot a random direction.
void randomDir() {
	long turnProb = 0;
	turnProb = rand() % 100;
	if(turnProb < 50) {
		turnLeft();
	} else {
		turnRight();
	}
}
int getGaussianSum (int num) {
	int sum = 0;
  for (;num > 0; num--) {
		sum += num;
  }
  return sum;
}

float getMemberOfMovingAvg (int i, int value, float gSum) {
	return value * ((i + 1) / gSum);
}

float getMovingAvg (float lastAvg, float alpha, int length, int newestReading) {
  return lastAvg + (alpha * (newestReading - lastAvg));
}

//this is for wondering right
void randomRight() {
	setMotorSpeed(leftMotor, 60);
	setMotorSpeed(rightMotor, 50);
}

//this is for wondering left
void randomLeft() {
	setMotorSpeed(leftMotor, 50);
	setMotorSpeed(rightMotor, 60);
}

//this is the random walking task.
//Lowest priority 2
task randomWalk() {
	//varibles that
	long rTime = 0;
	long turnProb = 0;
	long rightProbability = 50;
	long leftProbability = 50;


//this loop will let robot walking repeatly but with random pattern.
  while(true) {
  	//generates number between 0 and 99.
  	//starts with 50/50 chance of going left or right.
  	//lower end of probability will always be for turning left.
  	turnProb = rand() % 100;
  	rTime = rand() % 900 + 300;

	//random turning algorithm.
  	if(turnProb < leftProbability) {
  		randomLeft();
  		rightProbability += 10;
  		leftProbability -= 10;
  	} else {
  		randomRight();
  		rightProbability -= 10;
  		leftProbability += 10;
  	}
		wait1Msec(rTime);
	}
}


//Priority 1
//Kill wander task to follow line
//Black is approximately 10 +- 5
//White is approximately 70 +- 10
task lineFollow () {
	stopTask(randomWalk);

	//DO the right thing

	startTask(randomWalk);
}


//Priority 0
//Kill line detection to chase object and kill wander
//3 feet is approximately 90 from the getDistance function
task objectDetect () {
	//stopTask(lineFollow);
	int sonarAvg;
	while(true) {
		sonarAvg = getUSDistance(sonar);
		if(sonarAvg < 90 && sonarAvg > 45) {
			stopTask(randomWalk);
			setMotorSpeed(leftMotor, 100);
			setMotorSpeed(rightMotor, 100);
		} else if(sonarAvg < 45 && sonarAvg > 15) {
			setMotorSpeed(leftMotor, 35);
			setMotorSpeed(rightMotor, 35);
		}	else if(sonarAvg < 15){
			setMotorSpeed(leftMotor, 0);
			setMotorSpeed(rightMotor, 0);
			wait1Msec(2000);
			setMotorSpeed(leftMotor, -55);
			setMotorSpeed(rightMotor, -55);
			wait1Msec(200);
			randomDir();
			startTask(randomWalk);
		}
	}


	//startTask(lineFollow);

}

void setupAverages () {
	//initialize left/right color sensor average
	//init distance sensor average
	int colorAvgLeft [COLOR_SENSOR_AVERAGE_BUFFER_LEN];
	int colorAvgRight [COLOR_SENSOR_AVERAGE_BUFFER_LEN];
	int sonarAvg [SONAR_SENSOR_AVERAGE_BUFFER_LEN];

	for (int i = 0; i < COLOR_SENSOR_AVERAGE_BUFFER_LEN; i++) {
		colorAvgLeft[i] = getColorReflected(colorLeft);
		colorAvgRight[i] = getColorReflected(colorRight);
  }

  for (int i = 0; i < SONAR_SENSOR_AVERAGE_BUFFER_LEN; i++) {
		sonarAvg[i] = getUSDistance(sonar);
  }

  //Gauss(10) = 55
  float tmpSum = 0,
  			tmpSum2;

  int gSumColor = getGaussianSum(COLOR_SENSOR_AVERAGE_BUFFER_LEN),
      gSumSonar = getGaussianSum(SONAR_SENSOR_AVERAGE_BUFFER_LEN);

  for (int i = 0; i < COLOR_SENSOR_AVERAGE_BUFFER_LEN; i++) {
  	tmpSum += getMemberOfMovingAvg(i, colorAvgLeft[i], gSumColor);
		tmpSum2 += getMemberOfMovingAvg(i, colorAvgRight[i], gSumColor);
  }
  avgColorLeft = tmpSum;
  avgColorRight = tmpSum2;

  tmpSum = 0;
  for (int i = 0; i < SONAR_SENSOR_AVERAGE_BUFFER_LEN; i++) {
  	tmpSum += getMemberOfMovingAvg(i, sonarAvg[i], gSumSonar);
  }

  avgDist = tmpSum;


  writeDebugStreamLine("avg c l: %f, avg c r: %f, avg dist: %f", avgColorLeft, avgColorRight, avgDist);
}

task main()
{
	startTask(randomWalk);
	startTask(objectDetect);
	while (true);
	/*startTask(lineFollow);

	*/
}
