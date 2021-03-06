#ifndef _RIGIDBODY
#define _RIGIDBODY

#include <Eigen/Dense>
#include <iostream>
#include <set>
#include <vector>

class IndexRange {
 public:
  int start;
  int length;
  
  bool operator<(const IndexRange& other) const {
    return start<other.start;
  }
};

class RigidBodyManipulator;

using namespace Eigen;

class RigidBody {
public:
  RigidBody();
  ~RigidBody();

  void setN(int n);
  void computeAncestorDOFs(RigidBodyManipulator* model);

public:
  std::string linkname;
  std::string jointname;
  int robotnum; // uses 0-index. starts from 0
  static const std::set<int> defaultRobotNumSet;
// note: it's very ugly, but parent,dofnum,and pitch also exist currently (independently) at the rigidbodymanipulator level to represent the featherstone structure.  this version is for the kinematics.
  int parent;
  int dofnum;
  int floating;
  int pitch;
  MatrixXd contact_pts;
  Matrix4d Ttree;
  Matrix4d T_body_to_joint;

  std::set<int> ancestor_dofs;
  std::set<int> ddTdqdq_nonzero_rows;
  std::set<IndexRange> ddTdqdq_nonzero_rows_grouped;

  Matrix4d T;
  MatrixXd dTdq;
  MatrixXd dTdqdot;
  Matrix4d Tdot;
  MatrixXd ddTdqdq;

  double mass;
  Vector4d com;  // this actually stores [com;1] (because that's what's needed in the kinematics functions)

  friend std::ostream& operator<<( std::ostream &out, const RigidBody &b);
};




#endif
