
//import TrackerBox2;

public class TestMain {

	public static void main(String[] args) {
		TrackerBox2 tb = new TrackerBox2( "192.168.2.10");
//		tb.changeProfile(3);
		while(true) {
			try {
				TrackerBox2.ParticleReport pr = tb.getVisionData();
				System.out.println(pr);
				Thread.sleep(20);
			} catch(InterruptedException e){ }
		}
//		if (pr != null)
//			System.out.println(pr);
//		else
//			System.out.println("Communication Error");
			
//		tb.changeProfile(3);
	}
	

}
