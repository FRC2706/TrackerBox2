

import java.util.ArrayList;

public class TestMain {

		public static final String RPI_IP = "10.27.6.240";

		// public static final String RPI_IP = "127.0.0.1";


		public static void main(String[] args) {

			TrackerBox2 trackerbox = new TrackerBox2(RPI_IP);

			if (trackerbox == null)
				return;

			// // simalute a roboRIO asking for the data at 50 hz
			while (true) {
				ArrayList<TrackerBox2.TargetObject> targets = trackerbox.getVisionData();

				if (targets == null) {
					try {
						Thread.sleep(20);
						continue;
					} catch (InterruptedException e) {
						continue;
					}
				}

				System.out.println("I found "+targets.size()+" targets.");
				for(TrackerBox2.TargetObject target : targets)
					System.out.println("\tI found: "+target.toString());

				System.out.println();

				try {
					Thread.sleep(20);
				} catch (InterruptedException e) {
					continue;
				}
			}

			// speed test - how fast can we poll it?
			// long t1, t2;
			// while (true) {
			// 	t1 = System.currentTimeMillis();
			//
			// 	trackerbox.getVisionData();
			//
			// 	t2 = System.currentTimeMillis();
			//
			// 	System.out.println("I can ping it at: "+ 1000.0 / (t2-t1) +" requests / second.");
			//
			// }
		}

}
