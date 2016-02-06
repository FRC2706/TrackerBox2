package org.usfirst.frc.team2706.robot.vision;


public class TestMain {

	public static void main(String[] args) {
		new TrackerBox2(TrackerBox2.CAMERA_IP).getVisionData();
	}

}
