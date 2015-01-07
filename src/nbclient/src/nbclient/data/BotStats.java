package nbclient.data;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;

import nbclient.util.U;

public class BotStats {
	public static final int STAT_DATA_LENGTH = 417; //bytes
	public BotStats(Log log) throws IOException {
		assert(log.type().equalsIgnoreCase("stats"));
		assert(log.getAttributes().containsKey("nbuffers"));
		assert(log.bytes.length == STAT_DATA_LENGTH);
		
		int nb = NUM_LOG_BUFFERS = Integer.parseInt(log.getAttributes().get("nbuffers"));
		ByteArrayInputStream bais = new ByteArrayInputStream(log.bytes);
		DataInputStream is = new DataInputStream(bais);
		
		byte[] bmagic;
		String smagic;
		
		fio_stat = new BufStat[nb];
		cio_stat = new BufStat[nb];
		tot_stat = new BufStat[nb];
		
		manage = new BufManage[nb];
		ratio = new int[nb];
		size = new int[nb];
		
		U.w("start: " + (STAT_DATA_LENGTH - bais.available()) + " " + (STAT_DATA_LENGTH - is.available()));
		
		bmagic = new byte[4];
		is.readFully(bmagic);
		smagic = new String(bmagic);
		assert(smagic.equalsIgnoreCase("stat"));
		assert(bais.available() == is.available());
		
		U.w("after magic1: " + (STAT_DATA_LENGTH - bais.available()) + " " + (STAT_DATA_LENGTH - is.available()));
		
		for (int i = 0; i < nb; ++i) {
			BufStat b = fio_stat[i] = new BufStat();
			
			b.l_given = is.readInt();
			b.b_given = is.readLong();
			
			b.l_freed = is.readInt();
			b.l_lost = is.readInt();
			b.b_lost = is.readLong();
			
			b.l_writ = is.readInt();
			b.b_writ = is.readLong();
		}
		
		U.w("after fio: " + (STAT_DATA_LENGTH - bais.available()) + " " + (STAT_DATA_LENGTH - is.available()));
		
		for (int i = 0; i < nb; ++i) {
			BufStat b = cio_stat[i] = new BufStat();
			
			b.l_given = is.readInt();
			b.b_given = is.readLong();
			
			b.l_freed = is.readInt();
			b.l_lost = is.readInt();
			b.b_lost = is.readLong();
			
			b.l_writ = is.readInt();
			b.b_writ = is.readLong();
		}
		
		U.w("after cio: " + (STAT_DATA_LENGTH - bais.available()) + " " + (STAT_DATA_LENGTH - is.available()));
		
		for (int i = 0; i < nb; ++i) {
			BufStat b = tot_stat[i] = new BufStat();
			
			b.l_given = is.readInt();
			b.b_given = is.readLong();
			
			b.l_freed = is.readInt();
			b.l_lost = is.readInt();
			b.b_lost = is.readLong();
			
			b.l_writ = is.readInt();
			b.b_writ = is.readLong();
		}
		
		U.w("after tot: " + (STAT_DATA_LENGTH - bais.available()) + " " + (STAT_DATA_LENGTH - is.available()));
		
		for (int i = 0; i < nb; ++i) {
			BufManage m = manage[i] = new BufManage();
			m.servnr = is.readInt();
			m.filenr = is.readInt();
			m.nextw = is.readInt();
		}
		
		U.w("after manage: " + (STAT_DATA_LENGTH - bais.available()) + " " + (STAT_DATA_LENGTH - is.available()));
		
		bmagic = new byte[5];
		is.readFully(bmagic);
		smagic = new String(bmagic);
		U.w("[" +smagic + "]");
		assert(smagic.equalsIgnoreCase("magic"));
		
		fio_uptime = is.readLong();
		sio_uptime = is.readLong();
		
		con_uptime = is.readLong();
		cnc_uptime = is.readLong();
		
		log_uptime = is.readLong();
		
		for (int i = 0; i < nb; ++i) {
			ratio[i] = is.readInt();
		}
		
		for (int i = 0; i < nb; ++i) {
			size[i] = is.readInt();
		}
		
		cores = is.readInt();
		
		//FLAGS
		flags = new Flags();
		
		bmagic = new byte[5];
		is.readFully(bmagic);
		smagic = new String(bmagic);
		assert(smagic.equalsIgnoreCase("flags"));
		
		//set flags
		flags.serv_connected = (is.readByte() != 0);
		flags.cnc_connected = (is.readByte() != 0);
		
		flags.fileio = (is.readByte() != 0);
		flags.servio = (is.readByte() != 0);
		
		flags.STATS = (is.readByte() != 0);
		
		flags.SENSORS = (is.readByte() != 0);
		flags.GUARDIAN = (is.readByte() != 0);
		flags.COMM = (is.readByte() != 0);
		flags.LOCATION = (is.readByte() != 0);
		flags.ODOMETRY = (is.readByte() != 0);
		flags.OBSERVATIONS = (is.readByte() != 0);
		flags.LOCALIZATION = (is.readByte() != 0);
		flags.BALLTRACK = (is.readByte() != 0);
		flags.IMAGES = (is.readByte() != 0);
		flags.VISION = (is.readByte() != 0);
		
		bmagic = new byte[4];
		is.readFully(bmagic);
		smagic = new String(bmagic);
		assert(smagic.equalsIgnoreCase("endof"));
		
		is.close();
	}
	public int NUM_LOG_BUFFERS;
	public int cores;
	
	public int[] ratio;
	public int[] size;
	
	public BufStat[] fio_stat;
	public BufStat[] cio_stat;
	public BufStat[] tot_stat;
	
	public BufManage[] manage;
	
	public long con_uptime;
	public long cnc_uptime;
	
	public long fio_uptime;
	public long sio_uptime;
	
	public long log_uptime;
	
	public Flags flags;
	
	private final String manage_header = String.format("\t%10s%10s%10s\n", "servnr", "filenr", "nextw");
	private class BufManage {
		public int servnr;
		public int filenr;
		public int nextw;
		
		public String toString() {
			return String.format("\t%10d%10d%10d\n", servnr, filenr, nextw);
		}
	}
	
	private final String stat_header = String.format("\t%20s%20s%20s%20s%20s%20s%20s\n",
			"lgiven", "bgiven", "lwrit", "bwrit", "llost", "blost", "lfreed");
	private class BufStat {
		public int l_given;
		public int l_freed;
		public int l_lost;
		public int l_writ;
		
		public long b_given;
		public long b_lost;
		public long b_writ;
		
		public String toString() {
			return String.format("\t%20d%20d%20d%20d%20d%20d%20d\n",
					l_given, b_given, l_writ, b_writ, l_lost, b_lost, l_freed);
		}
	}

	private class Flags {
		boolean serv_connected;
        boolean cnc_connected;
        
        //log to
        boolean fileio;
        boolean servio;
        
        //what to log
        boolean STATS;
        
        //SPECIFIC modules
        boolean SENSORS;
        boolean GUARDIAN;
        boolean COMM;
        boolean LOCATION;
        boolean ODOMETRY;
        boolean OBSERVATIONS;
        boolean LOCALIZATION;
        boolean BALLTRACK;
        boolean IMAGES;
        boolean VISION;
        
        public String toString() {
        	return String.format("\tFlags:\n%20s %B\n%20s %B\n%20s %B\n%20s %B\n%20s %B\n" +
        			"%20s %B\n%20s %B\n%20s %B\n%20s %B\n%20s %B\n%20s %B\n%20s %B\n"
        			+ "%20s %B\n%20s %B\n", "serv_connected", serv_connected,
        			"cnc_connected", cnc_connected, "fileio", fileio, "servio", servio, "STATS", STATS,
        			"GUARDIAN", GUARDIAN, "COMM", COMM, "LOCATION", LOCATION, "ODOMETRY", ODOMETRY,
        			"OBSERVATIONS", OBSERVATIONS, "LOCALIZATION", LOCALIZATION, "BALLTRACK", BALLTRACK,
        			"IMAGES", IMAGES, "VISION", VISION);
        }
	}
	
	public String toString() {
		String[] parts = new String[7 + (5 * NUM_LOG_BUFFERS)];
		int pindex = 0;
		
		parts[pindex++] = String.format("stat_len=%d nbuffers=%d num_cores=%d\n", STAT_DATA_LENGTH, NUM_LOG_BUFFERS, cores);
		parts[pindex++] = String.format("\t%10s%10s\n", "ratio", "size");
		for (int i = 0; i < NUM_LOG_BUFFERS; ++i) {
			parts[pindex++] = i + String.format("\t%10d%10d\n", ratio[i], size[i]);
		}
		
		parts[pindex++] = stat_header;
		for (int i = 0; i < NUM_LOG_BUFFERS; ++i) {
			parts[pindex++] = "fio-" + i + fio_stat[i].toString();
		}
		
		for (int i = 0; i < NUM_LOG_BUFFERS; ++i) {
			parts[pindex++] = "cio-" + i + cio_stat[i].toString();
		}
		
		for (int i = 0; i < NUM_LOG_BUFFERS; ++i) {
			parts[pindex++] = "tot-" + i + tot_stat[i].toString();
		}
		
		parts[pindex++] = "\n" + manage_header;
		for (int i = 0; i < NUM_LOG_BUFFERS; ++i) {
			parts[pindex++] = i + manage[i].toString();
		}
		
		parts[pindex++] = "\ttiming:\n";
		parts[pindex++] = String.format("con-up=%d\ncnc-up=%d\nfio-up=%d\nsio-up=%d\nlog-up=%d\n",
				con_uptime, cnc_uptime, fio_uptime, sio_uptime, log_uptime);
		
		parts[pindex++] = flags.toString();
		
		int len = 0;
		for (String s : parts) len += s.length();
		StringBuilder buf = new StringBuilder(len);
		
		for (String s : parts) buf.append(s);
		return buf.toString();
	}
}
