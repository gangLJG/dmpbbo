/**
 * @file   TaskSolverDmp.cpp
 * @brief  TaskSolverDmp class source file.
 * @author Freek Stulp
 *
 * This file is part of DmpBbo, a set of libraries and programs for the 
 * black-box optimization of dynamical movement primitives.
 * Copyright (C) 2014 Freek Stulp, ENSTA-ParisTech
 * 
 * DmpBbo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * DmpBbo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with DmpBbo.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dmp_bbo/TaskSolverDmp.hpp"





#include <boost/serialization/export.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include "dmp_bbo/TaskSolverDmp.hpp"

BOOST_CLASS_EXPORT_IMPLEMENT(DmpBbo::TaskSolverDmp);

#include <boost/serialization/base_object.hpp>

#include <iostream>
#include <string>
#include <set>
#include <eigen3/Eigen/Core>

#include "utilities/EigenBoostSerialization.hpp"
#include "utilities/BoostSerializationToString.hpp"
#include "dmp/Dmp.hpp"

using namespace std;
using namespace Eigen;

namespace DmpBbo {
  
TaskSolverDmp::TaskSolverDmp(Dmp* dmp, set<string> optimize_parameters, double dt, double integrate_dmp_beyond_tau_factor, bool use_normalized_parameter)
: dmp_(dmp)
{
  dmp_->setSelectedParameters(optimize_parameters);
  
  integrate_time_ = dmp_->tau() * integrate_dmp_beyond_tau_factor;
  n_time_steps_ = (integrate_time_/dt)+1;
  use_normalized_parameter_ = use_normalized_parameter;
}
  
void TaskSolverDmp::performRollouts(const vector<MatrixXd>& samples, const MatrixXd& task_parameters, MatrixXd& cost_vars) const 
{
  // n_dofs-D Dmp, n_parallel=n_dofs
  //                   vector<Matrix>            Matrix
  // samples         = n_dofs x          n_samples x sum(nmodel_parameters_)
  // task_parameters =                   n_samples x n_task_pars
  // cost_vars       =                   n_samples x (n_time_steps*n_cost_vars)
  
  int n_dofs = samples.size();
  assert(n_dofs>0);
  assert(n_dofs==dmp_->dim_orig());
  
  int n_samples = samples[0].rows();
  for (int dd=1; dd<n_dofs; dd++)
  {
    assert(samples[dd].rows()==n_samples);
  }

  VectorXd ts = VectorXd::LinSpaced(n_time_steps_,0.0,integrate_time_);
  
  int n_cost_vars = 4*n_dofs+1;
  cost_vars.resize(n_samples,n_time_steps_*n_cost_vars); 
    
  vector<VectorXd> model_parameters_vec(n_dofs);
  for (int k=0; k<n_samples; k++)
  {
    for (int dd=0; dd<n_dofs; dd++)
      model_parameters_vec[dd] = samples[dd].row(k);
    dmp_->setModelParametersVectors(model_parameters_vec, use_normalized_parameter_);
    
    MatrixXd xs_ana;
    MatrixXd xds_ana;
    MatrixXd forcing_terms;
    dmp_->analyticalSolution(ts,xs_ana,xds_ana,forcing_terms);
    
    MatrixXd ys_ana;
    MatrixXd yds_ana;
    MatrixXd ydds_ana;
    dmp_->statesAsTrajectory(xs_ana,xds_ana,ys_ana,yds_ana,ydds_ana);
    
    int offset = 0;
    for (int tt=0; tt<n_time_steps_; tt++)
    {
      cost_vars.block(k,offset,1,n_dofs) = ys_ana.row(tt);   offset += n_dofs; 
      cost_vars.block(k,offset,1,n_dofs) = yds_ana.row(tt);  offset += n_dofs; 
      cost_vars.block(k,offset,1,n_dofs) = ydds_ana.row(tt); offset += n_dofs; 
      cost_vars.block(k,offset,1,1) = ts.row(tt);       offset += 1; 
      cost_vars.block(k,offset,1,n_dofs) = forcing_terms.row(tt); offset += n_dofs;       
    }
  }
}

string TaskSolverDmp::toString(void) const
{
  RETURN_STRING_FROM_BOOST_SERIALIZATION_XML("TaskSolverDmp");
}

}
