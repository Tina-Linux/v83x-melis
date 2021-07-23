sample_virvi测试流程：
         该sample测试mpi_vi组件,打开sample_virvi.conf指定的dev节点,mpi_vi采集图像,到达测试次数后,停止运行并销毁vi组件,可以手动按ctrl-c，终止测试。

允许不指定conf文件，直接运行：sample_virvi
    这样sample_virvi按照默认参数执行测试，默认参数参考函数loadSampleVirViConfig()，可以手动修改。
    参数：
    jpeg_width/jpeg_height：指定jpeg图片的目标宽高，可以大于或小于camera采集图像的宽高，这样就启动缩放编码。
    mJpegStoreCount: 指定jpeg文件数量。
	mJpegStoreInterval: 指定编码jpeg文件的帧间隔。

如果想指定conf文件，则如下操作：
读取测试参数的流程：
    sample提供了sample_virvi.conf，测试参数都写在该文件中。
启动sample_virvi时，在命令行参数中给出sample_virvi.conf的具体路径，sample_virvi会读取sample_virvi.conf，完成参数解析。
然后按照参数运行测试。
         从命令行启动sample_virvi的指令：
        ./sample_virvi -path /home/sample_virvi.conf
        "-path /home/sample_virvi.conf"指定了测试参数配置文件的路径。


测试参数的说明：
(1)auto_test_count:指定自动测试次数
(2)get_frame_count:指定每次测试获取图像的次数
(3)dev_num：指定VI Dev设备节点 
(4)pic_width：指定camera采集的图像宽度
(5)pic_height：指定camera采集的图像高度
(6)frame_rate:指定camera采集图像的帧率
(7)pic_format:指定camera采集图像像素格式
(8)jpeg_width：编码Jpeg图片的目标宽度
(9)jpeg_height：编码Jpeg图片的目标高度