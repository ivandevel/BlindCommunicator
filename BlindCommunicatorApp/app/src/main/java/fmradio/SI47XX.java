package fmradio;


import android.util.Log;


/***/
public class SI47XX {
    /* Constants */
    private static final String LogTag = "LogTagI2C";
    public static final String nodeNameDefault = "/dev/i2c-2";
    public static final String[] hin1 = { "su", "-c","chmod 777 /dev/i2c-2" };  //gain root access

    /* Variables */
    private I2C i2c;
    private String nodeName;

    int[] cmd = new int[8];
    int[] resp = new int[15];

    int SI4705_7BIT_ADDR = 0x11;
    int silabs_volume = 60;

    /***/
    public SI47XX (String nodeName) {
        this.nodeName = nodeName;
		// try {
		// 	Runtime.getRuntime().exec(hin1);
		// } catch (IOException e) {
		// 	// TODO Auto-generated catch block
		// 	e.printStackTrace();
		// };
        i2c = new I2C();
    }

    void write_command(int buf[], int len) {
        write_si4705(buf, len);
        WaitReady();
    }

    int write_si4705(int buf[], int len) {
        int fileHandle = 0;
        int res = 0;
        try {
            fileHandle = i2c.open(this.nodeName);
        } catch(Exception e) {
            Log.w(LogTag, "Could not open I2C interface");
            res = -1;
        }

        res = i2c.write(fileHandle, SI4705_7BIT_ADDR, 0, buf, len);

        i2c.close(fileHandle);

        return res;
    }

    int read_si4705(int command[], int cmdlen, int response[], int resplen) {
        int fileHandle = 0;
        int res = 0;

        try {
            fileHandle = i2c.open(this.nodeName);
        } catch(Exception e) {
            Log.w(LogTag, "Could not open I2C interface");
            res = -1;
        }

        res = i2c.write(fileHandle, SI4705_7BIT_ADDR, 0, command, cmdlen);

        i2c.read(fileHandle, SI4705_7BIT_ADDR, response, resplen);

        i2c.close(fileHandle);

        return res;
    }

    public void Powerdown()
    {
        //POWER_DOWN
        //cmd[0]=0x11;
        // write_command(cmd, 1);
        SetVolume(0);
    }

    public void Powerup()
    {
        //POWER_UP
        cmd[0]=0x01;
        cmd[1]=0x10; //XOSC ON
        //cmd[1]=0x00; //XOSC OFF
        cmd[2]=0x05;
        write_command(cmd, 3);

        //FM_AGC_OVERRIDE
        cmd[0]=0x28;
        cmd[1]=0x01;
        cmd[2]=0x00;
        write_command(cmd, 3);

        //There is a debug feature that remains active in Si4704/05/3x-D60 firmware which can create periodic noise in
        //audio. Silicon Labs recommends you disable this feature by sending the following bytes (shown here in
        //hexadecimal form):
        //0x12 0x00 0xFF 0x00 0x00 0x00
        cmd[0]=0x12;
        cmd[1]=0x00;
        cmd[2]=0xFF;
        cmd[3]=0x00;
        cmd[4]=0x00;
        cmd[5]=0x00;
        write_command(cmd, 6);

        //FM_SEEK_FREQ_SPACING
        cmd[0]=0x12;
        cmd[1]=0x00;
        cmd[2]=0x14;
        cmd[3]=0x02;
        cmd[4]=0x00;
        cmd[5]=0x05;
        write_command(cmd, 6);

        //FM_SEEK_TUNE_RSSI_THRESHOLD
        cmd[0]=0x12;
        cmd[1]=0x00;
        cmd[2]=0x14;
        cmd[3]=0x04;
        cmd[4]=0x00;
        cmd[5]=0x01;
        write_command(cmd, 6);

        //FM_SEEK_TUNE_SNR_THRESHOLD
        cmd[0]=0x12;
        cmd[1]=0x00;
        cmd[2]=0x14;
        cmd[3]=0x03;
        cmd[4]=0x00;
        cmd[5]=0x01;
        write_command(cmd, 6);

        //Antena input
        cmd[0]=0x12;
        cmd[1]=0x00;
        cmd[2]=0x11;
        cmd[3]=0x07;
        cmd[4]=0x00;
        cmd[5]=0x01;
        write_command(cmd, 6);

        //FM_SEEK_BAND_BOTTOM
        cmd[0]=0x12;
        cmd[1]=0x00;
        cmd[2]=0x14;
        cmd[3]=0x00;
        cmd[4]=0x19;
        cmd[5]=0x00;
        write_command(cmd, 6);

        //FM_DEEMPHASIS
        cmd[0]=0x12;
        cmd[1]=0x00;
        cmd[2]=0x11;
        cmd[3]=0x00;
        cmd[4]=0x00;
        cmd[5]=0x01;
        write_command(cmd, 6);

        //REFCLK_PRESCALE
        cmd[0]=0x12;
        cmd[1]=0x00;
        cmd[2]=0x02;
        cmd[3]=0x02;
        cmd[4]=0x00;
        cmd[5]=0x01;
        write_command(cmd, 6);

        //REFCLK_FREQ
        cmd[0]=0x12;
        cmd[1]=0x00;
        cmd[2]=0x02;
        cmd[3]=0x01;
        cmd[4]=0x80;
        cmd[5]=0x00;
        write_command(cmd, 6);

    }

    public void ShortAntenna()
    {
        //Antena input
        cmd[0]=0x12;
        cmd[1]=0x00;
        cmd[2]=0x11;
        cmd[3]=0x07;
        cmd[4]=0x00;
        cmd[5]=0x01;
        write_si4705(cmd, 6);
        WaitReady();
    }

    public void LongAntenna()
    {
        //Antena input
        cmd[0]=0x12;
        cmd[1]=0x00;
        cmd[2]=0x11;
        cmd[3]=0x07;
        cmd[4]=0x00;
        cmd[5]=0x00;
        write_si4705(cmd, 6);
        WaitReady();
    }

    public void SetVolume(int volume)
    {
        //char parameters[6];
        cmd[0]=0x12;
        cmd[1]=0x00;
        cmd[2]=0x40;
        cmd[3]=0x00;
        cmd[4]=0x00;
        // parameters[5]=0x3F; // maximum
        cmd[5] = volume;
        write_command(cmd, 6);
    }

    public void VolumeUp()
    {
        if (++silabs_volume > 63) silabs_volume = 63;
        SetVolume(silabs_volume);
    }

    public void VolumeDn()
    {
        if (--silabs_volume < 0) silabs_volume = 0;
        SetVolume(silabs_volume);
    }

    public void SetFrequency(int channel_freq)
    {
        cmd[0]=0x20;
        cmd[1]=0x00;
        cmd[2]=(channel_freq&0xff00) >> 8;
        cmd[3]=(channel_freq&0x00ff);
        cmd[4]=0x00;

        write_command(cmd, 5);
    }

    public void StartSearchUp()
    {
        cmd[0] = 0x21;
        cmd[1] = 0x0C;

        write_si4705(cmd, 2);
    }

    public void StartSearchDn()
    {
        cmd[0] = 0x21;
        cmd[1] = 0x04;

        write_si4705(cmd, 2);
    }

    public int GetFrequency()
    {
        int pChannel_Freq=0;
        //unsigned short loop_counter = 0;
        //int[] Si47XX_fm_tune_status = new int[2];
        //int[] Si47XX_reg_data = new int[8];
        //unsigned char error_ind = 0;

        cmd[0]=0x22;
        cmd[1]=0x01;

        read_si4705(cmd, 2, resp, 8);

        pChannel_Freq = ((resp[2] << 8) | resp[3]);

        return pChannel_Freq;
    }

    public int GetReady()
    {
        int pStatus=0;
        //unsigned short loop_counter = 0;
        //int[] Si47XX_fm_tune_status = new int[2];
        //int[] Si47XX_reg_data = new int[8];
        //unsigned char error_ind = 0;

        cmd[0]=0x22;
        //Si47XX_fm_tune_status[1]=0x01;

        read_si4705(cmd, 1, resp, 1);

        pStatus = resp[0];

        return pStatus;

    }

    public void WaitReady()
    {
        int i=3000;
        // Loop until CTS is found or stop due to the counter running out.
        while ((i > 0) && (GetReady() & 0x80)==0)
        {
            i--;
        }
    }

    int GetCapacitance()
    {
        int pCapacitance=0;
        //unsigned short loop_counter = 0;
        //int[] Si47XX_fm_tune_status = new int[2];
        //int[] Si47XX_reg_data = new int[8];
        //unsigned char error_ind = 0;

        cmd[0]=0x22;
        cmd[1]=0x01;

        read_si4705(cmd, 2, resp, 8);

        pCapacitance =  resp[7];

        return pCapacitance;
    }

}