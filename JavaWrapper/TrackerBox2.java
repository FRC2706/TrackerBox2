
import java.io.*;
import java.net.*;
import java.util.ArrayList;

public class TrackerBox2 {
	public  String RPi_addr;
	public final  int visionDataPort = 1182;

	public boolean PRINT_STUFF = false;

	public class TargetObject {
		  public float boundingArea = -1;     // % of cam [0, 1.0]
		  //center of target
		  public float ctrX = -1;             // [-1.0, 1.0]
		  public float ctrY = -1;             // [-1.0, 1.0]
		  // the aspect ratio of the target we found. This can be used directly as a poor-man's measure of skew.
		  public float aspectRatio = -1;
/*		public String toString() {
			return ctrX + "," + ctrY + "," + boundingArea + "," + aspectRatio;
		}*/
	}

	public TrackerBox2(String raspberryPiAddress) {
		RPi_addr = raspberryPiAddress;
	}

	//@SuppressWarnings("deprecation")
	public  ArrayList<TargetObject> getVisionData() {
		ArrayList<TargetObject> prList = new ArrayList<>();

		if(PRINT_STUFF)
			System.out.println("Setting up Sockets");

		try (
			Socket sock = new Socket(RPi_addr, visionDataPort);

			PrintWriter outToServer = new PrintWriter(sock.getOutputStream(), true);

			// BufferedReader inFromServer = new BufferedReader( new InputStreamReader(sock.getInputStream()));
		) {
			if(PRINT_STUFF)
				System.out.println("Sending request to TrackerBox2 for vision data");
			outToServer.println(""); // basically send an empty message
			outToServer.flush();

			byte[] rawBytes = new byte[2048];
			try {
				// rawData = inFromServer.read();
				if( sock.getInputStream().read(rawBytes) < 0 ) {
					System.out.println("Something went wrong reading response from TrackerBox2");
					return null;
				}

				String rawData = new String(rawBytes);
				if(PRINT_STUFF)
					System.out.println("I got back: " + rawData);

				if(rawData.length() == 0) {
					prList.add(new TargetObject());
				}
				String[] targets = rawData.split(":");
				for(String target : targets) {
					String[] targetData = target.split(",");

					TargetObject pr = new TargetObject();
					pr.ctrX = Float.parseFloat(targetData[0]);
					pr.ctrY	= Float.parseFloat(targetData[1]);
					pr.aspectRatio = Float.parseFloat(targetData[2]);
					pr.boundingArea = Float.parseFloat(targetData[3]);

					if(PRINT_STUFF)
						System.out.println("Target found at: " + pr.ctrX + "," + pr.ctrY + ", and aspectRatio and boundingArea is: " + pr.aspectRatio + "," + pr.boundingArea);

					prList.add(pr);
				}

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
		if(PRINT_STUFF)
			System.out.println("Network call successful, returning not null data...");

		return prList;
	}



}
