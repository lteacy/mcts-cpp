/**
 * @file TreeNode.h
 * This file defines the mcts::UCTreeNode class.
 */
#ifndef MCTS_TREENODE_H
#define MCTS_TREENODE_H

#include <cmath>
#include <cassert>
#include <limits>
#include <stack>
#include <iostream>

/**
 * Namespace for all public functions and types defined in the MCTS library.
 */
namespace mcts {

/**
 * Used to generate small random numbers to break ties when selecting actions.
 */
const double EPSILON = 1e-6;

/**
 * Default discount factor for future rewards.
 */
const double DEFAULT_GAMMA = 0.9;

/**
 * Represents a node in a UCT tree. This provides the main data structure and
 * implementation of the UCT (Upper Confidence Tree) algorithm.
 * @tparam N_ACTIONS the number of actions in the action domain.
 */
template<int N_ACTIONS> class UCTreeNode
{
private: 

   /**
    * Array of pointers (one per action) to all children of this node.
    */
   UCTreeNode* vpChildren_i[N_ACTIONS];

   /**
    * True iff this is a leaf node with no children. If this variable is true
    * then all elements of the UCTreeNode::vpChildren_i array should be null,
    * otherwise they should all point to valid objects.
    */
   bool isLeaf_i;

   /**
    * Counts the number of time this node has been visited.
    */
   double nVisits_i;
   
   /**
    * Sum of all values received each time this node has been visited.
    */
   double totValue_i;

   /**
    * Discount factor for future rewards.
    */
   double gamma_i;

   /**
    * Selects the next child to explore using UCB.
    * @pre This must not be a leaf node.
    * @returns a pointer to the best child of this node to explore next.
    */
   UCTreeNode* selectChild()
   {
      //***********************************************************************
      // Ensure preconditions are meet: can't select a child, if this is a
      // leaf node.
      //***********************************************************************
      assert(!isLeaf_i);

      //***********************************************************************
      // Initialise selected node to null, and the best value found so far
      // to its lowest possible value.
      //***********************************************************************
      UCTreeNode* pSelected = 0;
      double bestValue = -std::numeric_limits<double>::max();

      //***********************************************************************
      // For each child node
      //***********************************************************************
      for(int k=0; k<N_ACTIONS; ++k)
      {
         //********************************************************************
         // Sanity check that the current child is not null (this should
         // never happen for non leaf nodes).
         //********************************************************************
         UCTreeNode* pCur = vpChildren_i[k]; // ptr to current child node
         assert(0!=pCur);

         //********************************************************************
         // Calculate the UCT value of the current child
         //********************************************************************
         double uctValue = pCur->totValue_i / (pCur->nVisits_i + EPSILON) +
            std::sqrt(std::log(nVisits_i+1) / (pCur->nVisits_i + EPSILON));

         //********************************************************************
         // Add a small random value to break ties
         //********************************************************************
         uctValue += static_cast<double>(rand())*EPSILON/RAND_MAX;

         //********************************************************************
         // If the current child is the best one encounted so far, make it
         // the selected node, and store its value for later comparsion.
         //********************************************************************
         if (uctValue >= bestValue)
         {
            pSelected = pCur;
            bestValue = uctValue;
         }

      } // for loop

      //***********************************************************************
      // Sanity check that the selected node is not null, and return it to
      // the calling function.
      //***********************************************************************
      assert(0!=pSelected);
      return pSelected;

   } // selectChild

   /**
    * If this is a leaf node, expands the tree by initalising this node's
    * children. If this is not a leaf node, no action is taken.
    * @post If this is a leaf node, a new child node will be constructed for
    * each possible action.
    */
   void expand()
   {
      //***********************************************************************
      // If this is not a leaf node, then we're done. There is nothing to do.
      //***********************************************************************
      if(!isLeaf_i)
      {
         return; 
      }

      //***********************************************************************
      // Otherwise, we set the leaf flag to false (to indicate that this is
      // no longer a leaf node) and construct a new child for each action.
      //***********************************************************************
      isLeaf_i = false;
      for (int k=0; k<N_ACTIONS; ++k)
      {
         vpChildren_i[k] = new UCTreeNode();
      }

   } // expand 

   /**
    * Returns an estimated value for a leaf node using the rollout policy.
    * @param[in] node the UCTreeNode whose rollout value is to be estimated.
    * @return the estimated rollout value for \c node.
    */
   double rollOut(const UCTreeNode* const node)
   {
      // ultimately a roll out will end in some value
      // assume for now that it ends in a win or a loss
      // and just return this at random
      return static_cast<double>(std::rand())/RAND_MAX;
   }

   /**
    * Updates the statistics for this node for a given observed value.
    * @param[in] value the observed value.
    */
   void updateStats(double value)
   {
      nVisits_i++;         // increment the number of visits
      totValue_i += value; // update the total value for all visits
   }

public:

   /**
    * Construct a new UCTreeNode leaf node.
    * @param[in] inGamma discount factor for future rewards.
    */
   UCTreeNode(double inGamma=DEFAULT_GAMMA)
      : isLeaf_i(true), nVisits_i(0), totValue_i(0), gamma_i(inGamma)
   {
      //***********************************************************************
      // Since this is a leaf node with no children, all child pointers
      // should be null
      //***********************************************************************
      for(int k=0; k<N_ACTIONS; ++k)
      {
         vpChildren_i[k] = 0;
      }

   }  // default constructor

   /**
    * Returns true iff this is a leaf node with no children.
    */
   bool isLeaf()
   {
      return isLeaf_i;
   }

   /**
    * Performs one iteration of the MCTS algorithm, taking this to be the root
    * node.
    * @post the depth of the current best path from this node to the top of
    * the tree will be expanded by one. The value of all nodes along this path
    * will also be updated.
    */
   void iterate()
   {
      //***********************************************************************
      // Create stack to hold the path visited on this iteration.
      // Initially this holds only the current node.
      //***********************************************************************
      std::stack<UCTreeNode*> visited;
      UCTreeNode* pCur = this;
      visited.push(this);

      //***********************************************************************
      // Transverse the highest value path from the current node, until
      // we hit a leaf node.
      //***********************************************************************
      while (!pCur->isLeaf())
      {
         pCur = pCur->selectChild();
         visited.push(pCur);
      }

      //***********************************************************************
      // Expand the current (leaf) node by depth 1, and select its best child.
      //***********************************************************************
      pCur->expand();
      pCur = pCur->selectChild();
      visited.push(pCur);

      //***********************************************************************
      // Estimate the value of the new leaf node using the rollout policy
      //***********************************************************************
      double value = rollOut(pCur);

      //***********************************************************************
      // Update the statistics for each node along the path using the
      // discounted value. 
      //***********************************************************************
      while(!visited.empty())
      {
         pCur->updateStats(value);   // update statistics
         pCur = visited.top(); // get the previous node in the path
         visited.pop();              // remove the previous node from the stack
         value *= gamma_i;           // discount the value
      }

   } // iterate

   /**
    * Returns the current best action for the next step.
    * @returns the index of the best action.
    */
   int bestAction()
   {
      //***********************************************************************
      // If this is a leaf node, we have no information to go on, so we
      // just return a random action.
      //***********************************************************************
      if(isLeaf_i)
      {
         return std::rand()%N_ACTIONS;
      }

      //***********************************************************************
      // Initialise selected action, and the best value found so far
      // to its lowest possible value.
      //***********************************************************************
      int selected = 0;
      double bestValue = -std::numeric_limits<double>::max();

      //***********************************************************************
      // For each child node
      //***********************************************************************
      for(int k=0; k<N_ACTIONS; ++k)
      {
         //********************************************************************
         // Sanity check that the current child is not null (this should
         // never happen for non leaf nodes).
         //********************************************************************
         UCTreeNode* pCur = vpChildren_i[k]; // ptr to current child node
         assert(0!=pCur);

         //********************************************************************
         // Calculate expected value
         //********************************************************************
         double expValue = pCur->totValue_i / (pCur->nVisits_i + EPSILON);

         //********************************************************************
         // Add a small random value to break ties
         //********************************************************************
         expValue += static_cast<double>(rand())*EPSILON/RAND_MAX;

         //********************************************************************
         // If the current child is the best one encounted so far, make it
         // the selected node, and store its value for later comparsion.
         //********************************************************************
         if (expValue >= bestValue)
         {
            selected = k;
            bestValue = expValue;
         }

      } // for loop

      //***********************************************************************
      // Return the index of the best value
      //***********************************************************************
      return selected;

   } // bestAction

   /**
    * Returns the expected value for this tree.
    */
   int vValue()
   {
      return totValue_i/nVisits_i;
   }

   /**
    * Returns the Q-value for a given action.
    * @param[in] action the index of the action whose value should be returned.
    * @pre \c action must be between 0 and \c N_ACTIONS (the number of actions).
    */
   int qValue(int action)
   {
      assert(0<=action);
      assert(N_ACTIONS>action);
      return vValue() + gamma_i * vpChildren_i[action]->vValue();
   }

   /**
    * Counts the number of nodes in the tree with this node as its root.
    */
   int numOfNodes()
   {
      //***********************************************************************
      // If this is a leaf node, then there is only one node in the tree
      //***********************************************************************
      if(isLeaf_i)
      {
         return 1;
      }

      //***********************************************************************
      // Otherwise, we have to count the number of nodes in each subtree
      //***********************************************************************
      int result = 1;
      for(int k=0; k<N_ACTIONS; ++k)
      {
         result += vpChildren_i[k]->numOfNodes();
      }
      return result;

   } // numOfNodes

   /**
    * Returns the maximum depth of the tree with this node as its root.
    * The \c parentDepth parameter specifies the depth of the parent node
    * for recursion purposes. When called externally with this node as the
    * true root of the tree, it is usually sufficient to leave this parameter
    * with its default value of 0.
    * @param[in] parentDepth depth of parent node.
    */
   int maxDepth(int parentDepth=0)
   {
      //***********************************************************************
      // If this is a leaf node, then the result is just the parent's depth
      // plus 1.
      //***********************************************************************
      if(isLeaf_i)
      {
         return parentDepth + 1;
      }

      //***********************************************************************
      // Otherwise, we return the maximum subtree depth plus 1.
      //***********************************************************************
      int maxDepth = 0;
      for(int k=0; k<N_ACTIONS; ++k)
      {
         int curDepth = vpChildren_i[k]->maxDepth(parentDepth+1);
         if(maxDepth < curDepth)
         {
            maxDepth = curDepth;
         }
      }

      return maxDepth;

   } // maxDepth

}; // class UCTreeNode

/**
 * Produces a string representation of a treeNode for diagnostic purposes.
 * Basically, just prints the vValue and the qValue for each action.
 */
template<int N_ACTIONS> std::ostream& operator<<
(
 std::ostream& out,
 UCTreeNode<N_ACTIONS> tree
)
{

   //**************************************************************************
   // Print the V value
   //**************************************************************************
   out << "[V=" << tree.vValue();
   
   //**************************************************************************
   // If this is a leaf node, then we're done.
   //**************************************************************************
   if(tree.isLeaf())
   {
      out << ']';
      return out;
   }

   //**************************************************************************
   // Otherwise, also print the q values for each action.
   //**************************************************************************
   for(int k=0; k<N_ACTIONS; ++k)
   {
      out << ",Q" << k << '=' << tree.qValue(k);
   }
   out << ']';

   return out;

} // operator <<

} // namespace mcts

#endif // MCTS_TREENODE_H
