import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.EOFException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class TcpServer {

    final static int PORT_NUM = 3500;

    public static void main(String[] args) throws Exception {
        try {
            // ServerSocket 생성
            ServerSocket serverSocket = new ServerSocket();
            serverSocket.bind(new InetSocketAddress(PORT_NUM));
            System.out.println("Starting tcp Server: " + PORT_NUM);
            System.out.println("[ Waiting ]\n");
            while(true) {
                Socket socket = serverSocket.accept();
                System.out.println("Connected " + socket.getLocalPort() + " Port, From " + socket.getRemoteSocketAddress().toString() + "\n");
                // Thread
                Server tcpServer = new Server(socket);
                tcpServer.start();
            }
        } catch (IOException io) {
            io.getStackTrace();
        }
    }

    public static class Server extends Thread {
        private Socket socket = null;
        static List<PrintWriter> list = Collections.synchronizedList(new ArrayList<PrintWriter>());
        BufferedReader in = null;
        PrintWriter out = null;

        public Server(Socket socket) {
            this.socket = socket;
            try {
                out = new PrintWriter(socket.getOutputStream());
                in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                list.add(out);
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        @Override
        public void run() {
            String name = "";
            try {
                name = in.readLine();
                System.out.println("[" + name + " 새연결생성]");
                sendAll("[" + name + "]님이 들어오셨습니다.");

                while (in != null) {
                    String inputMsg = in.readLine();
                    if("quit".equals(inputMsg)) break;
                    sendAll(name + ">>" + inputMsg);
                }
            } catch (IOException e) {
                System.out.println("[" + name + " 접속끊김]");
            } finally {
                sendAll("[" + name + "]님이 나가셨습니다");
                list.remove(out);
                try {
                    socket.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            System.out.println("[" + name + " 연결종료]");

            /*
            try {
                name = in.readLine();
                System.out.println();
                while (true) {
                    name = in.readLine();

                    // Socket에서 가져온 출력스트림
                    DataOutputStream dos = new DataOutputStream(this.socket.getOutputStream());

                    // Socket에서 가져온 입력스트림
                    DataInputStream dis = new DataInputStream(this.socket.getInputStream());

                    // read int
                    int receiveLength = dis.readInt();

                    // receive bytes
                    byte receiveByte[] = new byte[receiveLength];
                    dis.readFully(receiveByte, 0, receiveLength);
                    String receiveMessage = new String(receiveByte, "UTF-8");
                    System.out.println(socket.getRemoteSocketAddress().toString() + " : " + receiveMessage);
                    
                    // send bytes
                    byte[] sendBytes = receiveMessage.getBytes("UTF-8");
                    System.out.println(new String(sendBytes, "UTF-8"));
                    for(Server s : users) {
                        if(s.socket == this.socket) continue;
                        s.socket.getOutputStream().write(sendBytes);
                        s.socket.getOutputStream().flush();
                    }
                }
            } catch (EOFException e) {

            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                try {
                    if (this.socket != null) {
                        System.out.println("\n[ Socket close ]");
                        System.out.println("Disconnected :" + this.socket.getInetAddress().getHostAddress() + ":" + this.socket.getPort());
                        this.socket.close();
                    }
                } catch (Exception e) {}
            }
            */
        }

        private void sendAll (String s) {
            for (PrintWriter out: list) {
                if(out == this.out) continue;
                out.println(s);
                out.flush();
            }
        }
    }
}
