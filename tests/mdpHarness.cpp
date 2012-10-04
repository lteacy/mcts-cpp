/**
 * @file mdpHarness.cpp
 * Test harness for MCTS applied to simple MDP
 */
#include <exception>
#include <iostream>
#include "TreeNode.h"

/**
 * Test harness (not yet implmented).
 */
int main()
{
   std::cout << "Hello world! " << std::endl;

   try
   {
      //************************************************************************
      // Try instantiate a tree node
      //************************************************************************
      const int N_ACTIONS = 4;
      mcts::UCTreeNode<N_ACTIONS> tree;

      //************************************************************************
      // Try expanding the tree a few times
      //************************************************************************
      const int N_ITERATIONS = 10;
      for(int k=0; k<N_ITERATIONS; ++k)
      {
         std::cout << "tree: " << tree << std::endl;
         std::cout << "iteration: " << k << std::endl;
         tree.iterate();
      }
      std::cout << "tree: " << tree << std::endl;

      //************************************************************************
      // Log the best action, depth and number of nodes
      //************************************************************************
      int bestAction = tree.bestAction();
      std::cout << "Best Action: " << bestAction << std::endl;
      int nNodes = tree.numOfNodes();
      std::cout << "Number of Nodes: " << nNodes << std::endl;
      int maxDepth = tree.maxDepth();
      std::cout << "Max Depth: " << maxDepth << std::endl;

      //************************************************************************
      // Check that the number of nodes is correct (this at least is
      // predicable, because we expand by N_ACTIONS on each iteration).
      //************************************************************************
      const int EXP_N_NODES = 1 + N_ACTIONS*N_ITERATIONS;
      if(EXP_N_NODES != nNodes)
      {
         std::cout << "Unexpected number of nodes. Should be: " <<
            EXP_N_NODES << std::endl;
         return EXIT_FAILURE;
      }
      else
      {
         std::cout << "Number of nodes is correct: " <<
            EXP_N_NODES << std::endl;
      }

   }
   catch(std::exception& e)
   {
      std::cout << "Caught error: " << e.what() << std::endl;
      return EXIT_FAILURE;
   }

   //***************************************************************************
   // Return sucessfully
   //***************************************************************************
   return EXIT_SUCCESS;
}

