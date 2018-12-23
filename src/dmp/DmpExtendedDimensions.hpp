/**
 * @file DmpExtendedDimensions.hpp
 * @brief  DmpExtendedDimensions class header file.
 * @author Freek Stulp
 *
 * This file is part of DmpExtendedDimensionsBbo, a set of libraries and programs for the 
 * black-box optimization of dynamical movement primitives.
 * Copyright (C) 2014 Freek Stulp, ENSTA-ParisTech
 * 
 * DmpExtendedDimensionsBbo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * DmpExtendedDimensionsBbo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with DmpExtendedDimensionsBbo.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _DMP_EXTENDED_DIMENSIONS_H_
#define _DMP_EXTENDED_DIMENSIONS_H_

// This must be included before any Eigen header files are included
#include "eigen_realtime/eigen_realtime_check.hpp"

#include "dmp/Dmp.hpp"

#include "dmpbbo_io/EigenBoostSerialization.hpp"
#include <boost/serialization/assume_abstract.hpp>

/*
Alternative: move into Dmp

Dmp::setFunctionApproximatorsExtendedDims()
Dmp::trainExtendedDims()
Dmp::analyticalSolutionExtendedDims(const Eigen::VectorXd& ts, Trajectory& trajectory) const;
Dmp::analyticalSolutionExtendedDims(const Eigen::VectorXd& ts, Eigen::MatrixXd& xs, Eigen::MatrixXd& xds, Eigen::MatrixXd& fa_extended_output) const;
Dmp::trainExtendedDims(const Trajectory& trajectory);
Dmp::trainExtendedDims(const Trajectory& trajectory, std::string save_directory, bool overwrite=false);
Dmp::predictExtendedDims(const Eigen::VectorXd& phases, Eigen::MatrixXd& output_extended_dims)
*/

namespace DmpBbo {
  
// forward declaration
class FunctionApproximator;
class Trajectory;

/** \defgroup Dmps Dynamic Movement Primitives
 */

/** 
 * \brief Implementation of Dynamical Movement Primitives with extended dimensions
 * \ingroup DmpExtendedDimensionss
 */
class DmpExtendedDimensions : public Dmp
{
public:
  
 DmpExtendedDimensions(
   Dmp* dmp,
   std::vector<FunctionApproximator*> function_approximators_extended
   );
   
  
  /** Destructor. */
  ~DmpExtendedDimensions(void);
  
  /** Return a deep copy of this object 
   * \return A deep copy of this object
   */
  DmpExtendedDimensions* clone(void) const;
  
  void integrateStart(Eigen::Ref<Eigen::VectorXd> x, Eigen::Ref<Eigen::VectorXd> xd, Eigen::Ref<Eigen::VectorXd> extended_dims) const;

  void integrateStep(double dt, const Eigen::Ref<const Eigen::VectorXd> x, Eigen::Ref<Eigen::VectorXd> x_updated, Eigen::Ref<Eigen::VectorXd> xd_updated, Eigen::Ref<Eigen::VectorXd> extended_dimensions) const;
  
  /**
   * Return analytical solution of the system at certain times (and return forcing terms)
   *
   * \param[in]  ts  A vector of times for which to compute the analytical solutions
   * \param[out] xs  Sequence of state vectors. T x D or D x T matrix, where T is the number of times (the length of 'ts'), and D the size of the state (i.e. dim())
   * \param[out] xds Sequence of state vectors (rates of change). T x D or D x T matrix, where T is the number of times (the length of 'ts'), and D the size of the state (i.e. dim())
   * \param[out] forcing_terms The forcing terms for each dimension, for debugging purposes only.
   * \param[out] fa_output The output of the function approximators, for debugging purposes only.
   * \param[out] fa_extended_output The output of the function approximators for the extended dimensions.
   *
   * \remarks The output xs and xds will be of size D x T \em only if the matrix x you pass as an argument of size D x T. In all other cases (i.e. including passing an empty matrix) the size of x will be T x D. This feature has been added so that you may pass matrices of either size. 
   */
  void analyticalSolution(const Eigen::VectorXd& ts, Eigen::MatrixXd& xs, Eigen::MatrixXd& xds, Eigen::MatrixXd& forcing_terms, Eigen::MatrixXd& fa_output, Eigen::MatrixXd& fa_extended_output) const;

  /**
   * Return analytical solution of the system at certain times
   *
   * \param[in]  ts  A vector of times for which to compute the analytical solutions
   * \param[out] trajectory The computed states as a trajectory.
   */
  void analyticalSolution(const Eigen::VectorXd& ts, Trajectory& trajectory) const;

  
  /**
   * Train a DMP with a trajectory.
   * \param[in] trajectory The trajectory with which to train the DMP.
   * \todo Document misc variables
   */
  void train(const Trajectory& trajectory);
      
  /**
   * Train a DMP with a trajectory, and write results to file
   * \param[in] trajectory The trajectory with which to train the DMP.
   * \param[in] save_directory The directory to which to save the results.
   * \param[in] overwrite Overwrite existing files in the directory above (default: false)
   * \todo Document misc variables
   */
  void train(const Trajectory& trajectory, std::string save_directory, bool overwrite=false);
  
  int dim_extended(void) const
  {
    return function_approximators_ext_dims_.size();
  }
  
  void getSelectableParameters(std::set<std::string>& selectable_values_labels) const;
  void setSelectedParameters(const std::set<std::string>& selected_values_labels);

  int getParameterVectorAllSize(void) const;
  void getParameterVectorAll(Eigen::VectorXd& values) const;
  void setParameterVectorAll(const Eigen::VectorXd& values);
  void getParameterVectorMask(const std::set<std::string> selected_values_labels, Eigen::VectorXi& selected_mask) const;

  
  /** Compute the outputs of the function approximators.
   * \param[in] phase_state The phase states for which the outputs are computed.
   * \param[out] fa_output The outputs of the function approximators.
   */
  virtual void computeFunctionApproximatorOutputExtendedDimensions(
    const Eigen::Ref<const Eigen::MatrixXd>& phase_state, Eigen::MatrixXd& fa_output) const;
  
  
protected:

  /** Get a pointer to the function approximator for a certain dimension.
   * \param[in] i_dim Dimension for which to get the function approximator
   * \return Pointer to the function approximator.
   */
  inline FunctionApproximator* function_approximator_extended_dimensions(int i_dim) const
  {
    assert(i_dim<(int)function_approximators_ext_dims_.size());
    return function_approximators_ext_dims_[i_dim];
  }
   
  
private:
  
  /** The function approximators, one for each extended dimension. */
  std::vector<FunctionApproximator*> function_approximators_ext_dims_;
  
  void initFunctionApproximatorsExtDims(std::vector<FunctionApproximator*> function_approximators);
  
  /** Pre-allocated memory to avoid allocating it during run-time. To enable real-time. */
  mutable Eigen::MatrixXd fa_ext_dim_outputs_one_prealloc_;
  
  /** Pre-allocated memory to avoid allocating it during run-time. To enable real-time. */
  mutable Eigen::MatrixXd fa_ext_dim_outputs_prealloc_;
  
  /** Pre-allocated memory to avoid allocating it during run-time. To enable real-time. */
  mutable Eigen::MatrixXd fa_ext_dim_output_prealloc_; 
  
protected:
   DmpExtendedDimensions(void) {};

private:
  /** Give boost serialization access to private members. */  
  friend class boost::serialization::access;
  
  /** Serialize class data members to boost archive. 
   * \param[in] ar Boost archive
   * \param[in] version Version of the class
   * See http://www.boost.org/doc/libs/1_55_0/libs/serialization/doc/tutorial.html#simplecase
   */
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version);

};

}

#include <boost/serialization/export.hpp>

/** Don't add version information to archives. */
BOOST_SERIALIZATION_ASSUME_ABSTRACT(DmpBbo::DmpExtendedDimensions);
 
/** Don't add version information to archives. */
BOOST_CLASS_IMPLEMENTATION(DmpBbo::DmpExtendedDimensions,boost::serialization::object_serializable);

#endif // _DMP_EXTENDED_DIMENSIONS_H_
