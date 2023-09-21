import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.Scanner;

public class TcpClient {

    final static int PORT_NUM = 3500;
    public static void main(String[] args) throws Exception {

        Socket socket = null;

        try {
            // Server와 통신하기 위한 Socket
            socket = new Socket();
            System.out.println("\n[ Request ... ]");
            
            // Server 접속
            socket.connect(new InetSocketAddress("localhost", PORT_NUM));
            System.out.println("\n[ Success ... ]");
            Thread sendThread = new SendThread(socket, Integer.toString(socket.getLocalPort()));
            sendThread.start();

            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            while (in != null) {
                String inputMsg = in.readLine();
                if(("[" + socket.getLocalPort() + "]님이 나가셨습니다").equals(inputMsg)) break;
                System.out.println("From:" + inputMsg);
            }
        } catch (IOException e) {
            System.out.println("[서버 접속끊김]");
        } finally {
            try {
                socket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        System.out.println("[서버 연결종료]");
        /*
            byte[] bytes = null;
            String message = null;

            // Socket에서 가져온 출력스트림
            DataOutputStream dos = new DataOutputStream(socket.getOutputStream());

            Scanner scanner = new Scanner(System.in);

            while(true) {
                // send bytes
                System.out.print(socket.getLocalPort() + " : ");
                message = scanner.nextLine();
                bytes = message.getBytes("UTF-8");
                dos.writeInt(bytes.length);
                dos.write(bytes, 0, bytes.length);
                dos.flush();
                // System.out.println("\n[ Data Send Success ]\n" + message);
                if("quit".equals(message)) break;

                // Socket에서 가져온 입력스트림
                DataInputStream dis = new DataInputStream(socket.getInputStream());

                // read int
                int receiveLength = dis.readInt();

                // receive bytes
                if (receiveLength > 0) {
                    byte receiveByte[] = new byte[receiveLength];
                    dis.readFully(receiveByte, 0, receiveLength);

                    message = new String(receiveByte, "UTF-8");
                    System.out.println("\n[ Data Receive Success ]\n" + message);
                }
            }

            socket.close();
            System.out.println("\n[ Socket close ]");
            
        } catch (Exception e) {
            e.printStackTrace();
        }

        if (!socket.isClosed()) {
            try {
                socket.close();
                System.out.println("\n[ Socket close ]");
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        */
    }

    public static class SendThread extends Thread {
        Socket socket = null;
        String name;
        Scanner scanner = new Scanner(System.in);

        public SendThread(Socket socket, String name) {
            this.socket = socket;
            this.name = name;
        }

        @Override
        public void run() {
            try {
                PrintStream out = new PrintStream(socket.getOutputStream());
                out.println(name);
                out.flush();

                while (true) {
                    String outputMsg = scanner.nextLine();
                    out.println(outputMsg);
                    out.flush();
                    if("quit".equals(outputMsg)) break;
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}
