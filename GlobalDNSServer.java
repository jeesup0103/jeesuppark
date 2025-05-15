import java.io.*;
import java.net.*;
import java.util.*;
public class GlobalDNSServer {
    private static final int PORT = 9000;
    private static final Map<String, String> db = new HashMap<>();
    private static void loadDB(String file) throws IOException {
        try (BufferedReader br = new BufferedReader(new FileReader(file))) {
            String line;
            while ((line = br.readLine()) != null) {
                String[] parts = line.trim().split("\\s+");
                if (parts.length == 2) db.put(parts[0], parts[1]);
            }
        }
    }
    public static void main(String[] args) throws Exception {
        loadDB("global_edu_dns.txt");
        loadDB("global_com_dns.txt");
        try (ServerSocket server = new ServerSocket(PORT, 50, InetAddress.getByName("127.0.0.1"))) {
            while (true) {
                Socket sock = server.accept();
                new Thread(() -> handle(sock)).start();
            }
        }
    }
    private static void handle(Socket sock) {
        try (BufferedReader in = new BufferedReader(new InputStreamReader(sock.getInputStream()));
             PrintWriter out = new PrintWriter(sock.getOutputStream(), true)) {
            char[] buf = new char[256];
            int n = in.read(buf);
            String domain = new String(buf, 0, n).trim();
            String ip = db.getOrDefault(domain, "Not Found");
            out.print(ip);out.flush();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try { sock.close(); } catch (IOException ignored) {}
        }
    }
}
