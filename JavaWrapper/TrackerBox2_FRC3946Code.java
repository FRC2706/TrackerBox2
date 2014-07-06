/**
 * This is the Java wrapper for Team 296's Raspberry Pi TrackerBox2.
 * Maintainer: Mike Ounsworth, ounsworth@gmail.com
 *
 * A special thanks to team 3946 for sharing their Java Socket code!
 */

import com.sun.squawk.util.StringTokenizer;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Vector;
import javax.microedition.io.Connector;
import javax.microedition.io.SocketConnection;

/**
 *
 * @author Gustave Michel
 * A TCP Socket Client
 */
public class TrackerBox2 {
    //Defaults
        public static final String ip = "10.2.96.28";
		public static final String changeProfilePort = "1181";
		public static final String getVisionDataPort = "1182";
//        public static final String defaultPort = "1180";
        public static final int bufferSize = 64;
        public static final char delimiter = ',';
    //Defaults
    
    private String changeProfileURL;
    private String getVisionDataURL;
    private boolean beenConnected = false;
    private boolean connected;
    
    private SocketConnection changeProfileSock;
    private SocketConnection getVisionDataSock;
    private InputStream cp_is;
    private InputStream gvd_is;
    private OutputStream cp_os;
    private OutputStream gvd_os;
    
    public static byte[] receiveData;
    private String rawData;
    
    /**
     * Constructor using Defaults
     */
    public TrackerBox2() {
        try {
            connect(ip, port, bufferSize, delimiter);
        } catch (IOException ex) {
            System.out.println("Failed to Connect, SocketPi Constructor");
        }
    }
    
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
	
    
    /**
     * Connect using last settings
     */
//    public void connect() throws IOException {
//        if(beenConnected == false) {
//            connect(defaultIp, defaultPort, defaultBufferSize, defaultDelimiter);
//        } else {
//            this.connect(ip, port, bufferSize, delimiter);
//        }
//    }
    /**
     * Connect using custom settings
     * @param ip Address of Server "xxx.xxx.xxx.xxx"
     * @param port of Server "XXXXX"
     */
    public void connect() throws IOException {
        beenConnected = true;
        changeProfileURL = "socket://" + ip + ":" + changeProfilePort; //Store URL of Connection
        getVisionDataURL = "socket://" + ip + ":" + getVisionDataPort; //Store URL of Connection
        System.out.println("Connecting to PI...");
        changeProfileSock = (SocketConnection) Connector.open(changeProfileURL, Connector.READ_WRITE, true); //Setup input and output through client SocketConnection
        getVisionDataSock = (SocketConnection) Connector.open(getVisionDataURL, Connector.READ_WRITE, true); //Setup input and output through client SocketConnection
        try{            
            cp_is = changeProfileSock.openInputStream();
            gvd_is = getVisionDataSock.openInputStream();
            cp_os = changeProfileSock.openOutputStream();
            gvd_os = getVisionDataSock.openOutputStream();
            if(true) {
                System.out.println("Connected to: "+changeProfileSock.getAddress() + ":" + changeProfileSock.getPort());
                System.out.println("and 		  "+getVisionDataSock.getAddress() + ":" + getVisionDataSOck.getPort());
            }
            connected = true;
        }
        catch (Exception ex){
             connected = false;
        }
        
        
    }
    
    /**
     * Closes socket connection
     */
    public void disconnect() throws IOException {
        cp_is.close();
        gvd_is.close();
        cp_os.close();
        gvd_os.close();
        changeProfileSock.close();
        getVisionDataSock.close();
        System.out.println("Disconnected");
        connected = false;
        
    }
    
    /**
     * Returns Status of Socket connection
     * @return if 
     */
    public boolean isConnected() {
        
        //need to actually test the connection 
        //to figure out if we're connected or not
        try{
            cp_os.write('\n'); //request Data
            gvd_os.write('\n');
        }
        catch(Exception ex){
            connected = false;
        }
        
        return connected;
    }
    
    
    public ParticleReport getVisionData() {
    	byte[] input;
        
        if (connected) {
            os.write('G'); //request Data
            System.out.println("Requested Data");
            
            if(is.available() <= bufferSize) {
                input = new byte[is.available()]; //storage space sized to fit!
                receiveData = new byte[is.available()];
                is.read(input);
                for(int i = 0; (i < input.length) && (input != null); i++) {
                    receiveData[i] = input[i]; //transfer input to full size storage
                }
            } else {
                System.out.println("PI OVERFLOW");
                is.skip(is.available()); //reset if more is stored than buffer
                return null;
            }
            
            rawData = ""; //String to transfer received data to
            System.out.println("Raw Data: "+receiveData.length);
            for (int i = 0; i < receiveData.length; i++) {
                rawData += (char) receiveData[i]; //Cast bytes to chars and concatinate them to the String
            }
            System.out.println(rawData);
            
            // parse the returned stuff into a ParticleReport
            String[] tokens = rawData.split(",");
            ParticleReport pr = new ParticleReport();
			pr.centerX 	= Double.parseDouble(tokens[0]);
			pr.centerY 	= Double.parseDouble(tokens[1]);
			pr.area 	= Double.parseDouble(tokens[2]);
			pr.velX 	= Double.parseDouble(tokens[3]);
			pr.velY		= Double.parseDouble(tokens[4]);
            
            return pr;
           
        } else {
            connect();
            return null;
        }
    }
    
    
    
    // TODO: change all thf
    public String getRawData() throws IOException {
        byte[] input;
        
        if (connected) {
            os.write('G'); //request Data
            System.out.println("Requested Data");
            
            if(is.available() <= bufferSize) {
                input = new byte[is.available()]; //storage space sized to fit!
                receiveData = new byte[is.available()];
                is.read(input);
                for(int i = 0; (i < input.length) && (input != null); i++) {
                    receiveData[i] = input[i]; //transfer input to full size storage
                }
            } else {
                System.out.println("PI OVERFLOW");
                is.skip(is.available()); //reset if more is stored than buffer
                return null;
            }
            
            rawData = ""; //String to transfer received data to
            System.out.println("Raw Data: "+receiveData.length);
            for (int i = 0; i < receiveData.length; i++) {
                rawData += (char) receiveData[i]; //Cast bytes to chars and concatinate them to the String
            }
            System.out.println(rawData);
            return rawData;
        } else {
            connect();
            return null;
        }
    }
    
    
    // TODO: change all this
    /**
     * Separates input String into many Strings based on the delimiter given
     * @param input String to be tokenized
     * @return String Array of Tokenized Input String
     */
    public String[] tokenizeData(String input) {
        StringTokenizer tokenizer = new StringTokenizer(input, String.valueOf(delimiter));
        String output[] = new String[tokenizer.countTokens()];
        
        for(int i = 0; i < output.length; i++) {
            output[i] = tokenizer.nextToken();
        }
        return output;
    }
    
    public Vector parseData(String[] input) {
        System.out.println("Parse Input: "+input.length);
        Vector output = new Vector(input.length);
        for(int i = 0; i < input.length; i++) {
            System.out.print(i+" ");
            if(isNumeric(input[i])) {
                output.setElementAt(new Integer(Integer.parseInt(input[i])), i); //Convert to Int if possible
            } else {
                //TODO Check for single character and convert to Character
                output.setElementAt(input[i], i); //Store as is
            }
        }
        return output;
    }
    
    public boolean isNumeric(String input) {
        try {
            Double.parseDouble(input);
            return true;
        } catch(NumberFormatException e) {
            return false;
        }
    }
    
    public void setDelimiter(char delimiter) {
        this.delimiter = delimiter;
    }
}
