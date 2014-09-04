#include <iostream>
#include <pcl/point_types.h>
#include <pcl/common/transforms.h>
#include <pcl/registration/transformation_estimation_svd.h>
#include <pcl/registration/transformation_estimation_svd_scale.h>
#include <pcl/common/centroid.h>
#include <boost/random.hpp>




int main (int argc, char** argv)
{
  
  bool use_scale = true;
  
  
  // alternative to rand()
  boost::random::mt19937 gen ( std::time(0) ); // random seed with current time in second
  boost::random::uniform_real_distribution<float> frand( 1.0, 3.2 ); // random gen between 1.0 and 3.2
  
  
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_source ( new pcl::PointCloud<pcl::PointXYZ> () );
  pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_target ( new pcl::PointCloud<pcl::PointXYZ> () );
  
  // create random source point cloud
  for (int i = 0; i < 10; i++) {
    cloud_source->push_back (pcl::PointXYZ (frand(gen), frand(gen), frand(gen) ));
  }
  

  // create random transformation: R and T
  Eigen::Affine3f transformation_true;
  {
    // random rotation matrix
    Eigen::Vector3f axis;
    axis.setRandom().normalize();
    float angle = frand( gen );
    Eigen::Affine3f R ( Eigen::AngleAxis<float> ( angle, axis ) );
    
    // random translation vector
    Eigen::Translation3f T ( frand(gen), frand(gen), frand(gen) );

    std::cout << "true R" << std::endl << R.matrix() << std::endl
              << "true T" << std::endl << T .vector() << std::endl;
    std::cout << "|R| " << R.matrix().topLeftCorner(3,3).determinant() << std::endl;

    if ( use_scale )
    {
      float scale = frand( gen );
      R.matrix().topLeftCorner(3,3) *= scale;
      std::cout << "true sR" << std::endl << R.matrix() << std::endl
                << "true scale " << scale << std::endl;
    }
    std::cout << "|R| " << R.matrix().topLeftCorner(3,3).determinant() << std::endl;
    // R and T
    transformation_true = T * R ; // shoul be in this order if you mean (Rx + T).   If R*T, then R(x+t) !

  }

  // create target point cloud
  pcl::transformPointCloud ( *cloud_source, *cloud_target, transformation_true );
    
  boost::shared_ptr< pcl::registration::TransformationEstimation< pcl::PointXYZ, pcl::PointXYZ > > estPtr;
  if ( use_scale )
    estPtr.reset ( new pcl::registration::TransformationEstimationSVDScale < pcl::PointXYZ, pcl::PointXYZ > () ); // estimator of R and T along with scale
  else 
    estPtr.reset ( new pcl::registration::TransformationEstimationSVD < pcl::PointXYZ, pcl::PointXYZ > () ); // estimator of R and T

    
  Eigen::Affine3f transformation_est;
  estPtr->estimateRigidTransformation ( *cloud_source,
                                    *cloud_target,
                                    transformation_est.matrix() );
  
  std::cout << "true transformation" << std::endl << transformation_true.matrix() << std::endl;
  std::cout << "estimated transformation " << std::endl << transformation_est.matrix()  << std::endl;
  if ( use_scale ) {
    Eigen::Matrix3f R = transformation_est.matrix().topLeftCorner(3,3);
    std::cout << "estimated scale " << std::sqrt( (R.transpose() * R).trace() / 3.0 ) << std::endl;
  }
  
  return ( 0 );
}
