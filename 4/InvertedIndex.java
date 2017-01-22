import java.io.IOException;
import java.util.*;
import java.lang.*;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.mapreduce.Job;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.Reducer;
import org.apache.hadoop.mapreduce.lib.input.FileInputFormat;
import org.apache.hadoop.mapreduce.lib.input.FileSplit;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;
import org.apache.hadoop.util.GenericOptionsParser;
public class InvertedIndex{

    public static void main(String[] args)throws Exception{
	Configuration conf = new Configuration();
        String[] otherArgs = new GenericOptionsParser(conf, args).getRemainingArgs();
        if (otherArgs.length < 2) {
          System.err.println("Usage: invertedindex <in> [<in>...] <out>");
          System.exit(2);
        }
	Job job = Job.getInstance(conf, "inverted index");
        job.setJarByClass(InvertedIndex.class);
        job.setMapperClass(MyMapper.class);
        job.setCombinerClass(MyCombiner.class);
        job.setReducerClass(MyReducer.class);
        job.setOutputKeyClass(Text.class);
        job.setOutputValueClass(Text.class);
        for (int i = 0; i < otherArgs.length - 1; ++i) {
          FileInputFormat.addInputPath(job, new Path(otherArgs[i]));
        }
        FileOutputFormat.setOutputPath(job,
          new Path(otherArgs[otherArgs.length - 1]));
        System.exit(job.waitForCompletion(true) ? 0 : 1);
    }
    
    public static class MyMapper extends Mapper<Object, Text, Text, Text>{
        private FileSplit mySplit;
	private Text valueItem = new Text();
	private Text keyItem = new Text();
	public void map(Object key, Text value, Context context)throws IOException, InterruptedException{
	    StringTokenizer iter = new StringTokenizer(value.toString());
	    mySplit = (FileSplit)context.getInputSplit();
	    while(iter.hasMoreTokens()){
		valueItem.set("1");
		keyItem.set(iter.nextToken() + ">" + mySplit.getPath().toString());
		context.write(keyItem, valueItem);
	    }
	}
    }

    public static class MyCombiner extends Reducer<Text, Text, Text, Text>{
        public void reduce(Text key, Iterable<Text> valueList, Context context)throws IOException, InterruptedException{
	    int counter = 0;
	    int spliter = key.toString().indexOf(">");
	    for(Iterator<Text> iter = valueList.iterator();iter.hasNext();){
		Text value = iter.next();
		counter += 1;
	    }
	    valueItem.set(key.toString().substring(spliter+1) + ":" + counter);
	    key.set(key.toString().substring(0,spliter));
	    context.write(key, valueItem);
	}
	private Text valueItem = new Text();
    }

    public static class MyReducer extends Reducer<Text, Text, Text, Text>{
	private Text valueItem = new Text();
	public void reduce(Text key, Iterable<Text> valueList, Context context)throws IOException, InterruptedException{
	    String result = new String();
	    for(Iterator<Text> iter = valueList.iterator();iter.hasNext();){
		Text value = iter.next();
		result = result + value.toString()+",";
	    }
	    valueItem.set(result);
	    context.write(key, valueItem);
	}
    }
}
