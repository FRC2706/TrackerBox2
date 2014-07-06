/**
 * Written for the FIRST Robotics Competition
 * Copyright 2014 Mike Ounsworth
 * ounsworth@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import java.io.*;
import java.net.*;

public class TrackerBox2 {

	public final String RPi_addr;
	public final int changeProfilePort = 1181;
	public final int getVisionDataPort = 1182;

	class ParticleReport {
		double centerX; // % of screen
		double centerY; // % of screen
		double area; // % of screen
		double velX;
		double velY;
		
		public String toString() {
			return "center: ("+centerX+","+centerY+")\narea: "+area+"\nvelocity: ("+velX+","+velY+")";
		}
	}
	
	public TrackerBox2(String raspberryPiAddress) {
		RPi_addr = raspberryPiAddress;
	}
	
	public void changeProfile(int newProfileNum) {
		try{
			Socket sock = new Socket(RPi_addr, changeProfilePort);
			
			OutputStream outToServer = sock.getOutputStream();
			
			DataOutputStream out = new DataOutputStream(outToServer);
			
			System.out.println("Sending request to switch to Profile "+Integer.toString(newProfileNum));
			out.writeUTF( Integer.toString(newProfileNum) );
			
			sock.close();
		} catch ( UnknownHostException e) {
			System.out.println("Host unknown: "+RPi_addr);
			return;
		} catch (java.net.ConnectException e) {
			System.out.println("TrackerBox Raspberry Pi is either not connected, or is not at address " + RPi_addr);
			return;
		} catch( IOException e) {
			e.printStackTrace();
			return;
		}
		
	}
	
	public ParticleReport getVisionData() {
		ParticleReport pr = new ParticleReport();
		try{
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
				String[] tokens = rawData.split(",");
				pr.centerX 	= Double.parseDouble(tokens[0]);
				pr.centerY 	= Double.parseDouble(tokens[1]);
				pr.area 	= Double.parseDouble(tokens[2]);
				pr.velX 	= Double.parseDouble(tokens[3]);
				pr.velY		= Double.parseDouble(tokens[4]);
//				System.out.println("ParticleReport:\n" + pr);
			} catch (java.io.EOFException e) {
				System.out.println("TrackerBox2: Communication Error");
			}
			
			sock.close();
		} catch ( UnknownHostException e) {
			System.out.println("Host unknown: "+RPi_addr);
			return null;
		} catch (java.net.ConnectException e) {
			System.out.println("TrackerBox Raspberry Pi is either not connected, or is not at address " + RPi_addr);
			return null;
		} catch( IOException e) {
			e.printStackTrace();
			return null;
		}
		return pr;
	}


}
