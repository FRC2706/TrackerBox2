package org.usfirst.frc.team2706.robot.vision;
import java.io.*;
import java.net.*;
import java.util.ArrayList;

public class TrackerBox2 {
	public static final String CAMERA_IP = "10.27.6.231";
	public static final float DEFAULT_PAN = 0.5f;
	public static final float DEFAULT_TILT = 1;
	public  String RPi_addr;
	public final int changeProfilePort = 1181;
	public final  int getVisionDataPort = 1182;
	public class TargetObject {
		  public float boundingArea = -1;     // % of cam [0, 1.0]
		  //center of target
		  public float ctrX = DEFAULT_PAN;             // [-1.0, 1.0]
		  public float ctrY = DEFAULT_TILT;             // [-1.0, 1.0]
		  // the aspect ratio of the target we found. This can be used directly as a poor-man's measure of skew.
		  public float aspectRatio = -1;
/*		public String toString() {
			return ctrX + "," + ctrY + "," + boundingArea + "," + aspectRatio;
		}*/
	}
	
	public TrackerBox2(String raspberryPiAddress) {
		RPi_addr = raspberryPiAddress;
	}
	@SuppressWarnings("deprecation")
	public  ArrayList<TargetObject> getVisionData() {
		ArrayList<TargetObject> prList = new ArrayList<>(); 
		try{
			System.out.println("testing");
			Socket sock = new Socket(RPi_addr, getVisionDataPort);
			
			OutputStream outToServer = sock.getOutputStream();
			
			DataOutputStream out = new DataOutputStream(outToServer);
			
//			System.out.println("Sending request to TrackerBox2 for vision data");
			out.writeUTF( " " ); // basically send an empty message
			
			String rawData = "";
			DataInputStream in = new DataInputStream(sock.getInputStream());
			try {
				rawData = in.readLine();
//				System.out.println("I got back: " + rawData);
				if(rawData.length() == 0) {
					prList.add(new TargetObject());
				}
				String[] targets = rawData.split(":");
				System.out.println(rawData);
				for(String target : targets) {
					TargetObject pr = new TargetObject();
					String[] targetData = target.split(",");
					pr.ctrX = Float.parseFloat(targetData[0]);
					pr.ctrY	= Float.parseFloat(targetData[1]);
					pr.aspectRatio = Float.parseFloat(targetData[2]);
					pr.boundingArea = Float.parseFloat(targetData[3]);
					System.out.println("Network call finished, current location is: " + pr.ctrX + "," + pr.ctrY + ", and aspectRatio and boundingArea is: " + pr.aspectRatio + "," + pr.boundingArea);
					prList.add(pr);
				}

//				System.out.println("ParticleReport:\n" + pr);
			} catch (java.io.EOFException e) {
				System.out.println("Camera: Communication Error");
			}
			
			sock.close();
		} catch ( UnknownHostException e) {
			System.out.println("Host unknown: "+RPi_addr);
			return null;
		} catch (java.net.ConnectException e) {
			System.out.println("Camera initialization failed at: " + RPi_addr);
			return null;
		} catch( IOException e) {
			e.printStackTrace();
			return null;
		}
		System.out.println("Network call successful, returning not null data...");
		return prList;
	}



}
