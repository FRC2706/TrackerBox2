


public class TestMain {

		// public static final String RPI_IP = "10.27.6.231";

		public static final String RPI_IP = "127.0.0.1";


		public static void main(String[] args) {

			TrackerBox2 trackerbox = new TrackerBox2(RPI_IP);
			// simalute a roboRIO asking for the data at 50 hz
			while (true) {
				trackerbox.getVisionData();

				try {
					Thread.sleep(20);
				} catch (InterruptedException e) {
					continue;
				}
			}
		}

}
