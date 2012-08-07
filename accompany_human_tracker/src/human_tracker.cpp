
#include <ros/ros.h>
#include <accompany_human_tracker/HumanLocations.h>
#include <accompany_human_tracker/TrackedHumans.h>

#include <MyTracker.h>
#include <TimTracker/Tracker.h>

#include <iostream>
using namespace std;


#define MY_TRACKER	1
#define TIM_TRACKER	2
#define TRACKER TIM_TRACKER // select on of the trackers above

// globals
ros::Publisher trackedHumansPub;
ros::Time prevNow;

#if TRACKER == MY_TRACKER
MyTracker myTracker;
#elif TRACKER == TIM_TRACKER
Tracker tracker;
#endif

void humanLocationsReceived(const accompany_human_tracker::HumanLocations::ConstPtr& humanLocations)
{
  /*for (unsigned int i=0;i<humanLocations->locations.size();i++)
  {
    cout<<"humanLocations["<<i<<"].x="<<humanLocations->locations[i].x<<endl;
    cout<<"humanLocations["<<i<<"].y="<<humanLocations->locations[i].y<<endl;
    cout<<"humanLocations["<<i<<"].z="<<humanLocations->locations[i].z<<endl;
  }
  */
#if TRACKER == MY_TRACKER
  // using simple MyTracker
  //myTracker.trackHumans(humanLocations);
  //trackedHumansPub.publish(myTracker.getTrackedHumans());
#elif TRACKER == TIM_TRACKER
  // using TimTracker
  vector<Tracker::TrackPoint> trackPoints;
  for (unsigned int i=0;i<humanLocations->locations.size();i++)
  {
    Tracker::TrackPoint point = {humanLocations->locations[i].x,
                                 humanLocations->locations[i].y,
                                 0.01,
                                 0.01};
    trackPoints.push_back(point);
  }
  double deltaTime=0;
  ros::Time now=ros::Time::now();
  ros::Duration duration=now-prevNow;
  deltaTime=duration.toSec();
  prevNow=now;
  cout<<"trackPoints.size(): "<<trackPoints.size()<<endl;
  tracker.update(trackPoints, deltaTime);

  accompany_human_tracker::TrackedHumans trackedHumans;
  cout<<"tracks.size(): "<<tracker.tracks.size()<<endl;
  for (vector<Tracker::Track>::iterator it=tracker.tracks.begin();it!=tracker.tracks.end();it++)
  {
    cv::Mat mat=it->filter.getState();
    accompany_human_tracker::TrackedHuman trackedHuman;
    trackedHuman.location.x=mat.at<float>(0,0);
    trackedHuman.location.y=mat.at<float>(1,1);
    trackedHuman.location.z=0;
    trackedHuman.id=it->id;
    trackedHumans.trackedHumans.push_back(trackedHuman);
  }
  trackedHumansPub.publish(trackedHumans);
#endif
}

int main(int argc,char **argv)
{
  ros::init(argc, argv, "HumanTracker");

  // create publisher and subscribers
  ros::NodeHandle n;
  prevNow=ros::Time::now();
  trackedHumansPub=n.advertise<accompany_human_tracker::TrackedHumans>("/trackedHumans",10);
  ros::Subscriber humanLocationsSub=n.subscribe<accompany_human_tracker::HumanLocations>("/humanLocations",10,humanLocationsReceived);
  ros::spin();

  return 0;
}
