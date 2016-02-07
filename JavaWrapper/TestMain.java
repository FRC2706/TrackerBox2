


public class TestMain {

		// public static final String RPI_IP = "10.27.6.231";

		public static final String RPI_IP = "127.0.0.1";


		public static void main(String[] args) {
			new TrackerBox2(RPI_IP).getVisionData();
		}

}
