/******************************************************************************
CryptoMiniSat -- Copyright (c) 2011 Mate Soos

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#ifndef __SOLUTIONEXTENDER_H__
#define __SOLUTIONEXTENDER_H__

#include "SolverTypes.h"
#include "Clause.h"

#ifdef VERBOSE_DEBUG
#define VERBOSE_DEBUG_RECONSTRUCT
#endif

//#define VERBOSE_DEBUG_RECONSTRUCT

class ThreadControl;

class SolutionExtender
{
    class MyClause
    {
        public:
            MyClause(const vector<Lit>& _lits, const bool _isXor, const bool _rhs = true) :
                lits(_lits)
                , rhs(_rhs)
            {
            }

            MyClause(const Clause& cl) :
                rhs(true)
            {
                for (const Lit *l = cl.begin(), *end = cl.end(); l != end; l++) {
                    lits.push_back(*l);
                }
            }

            MyClause(const Lit lit1, const Lit lit2) :
                rhs(true)
            {
                lits.push_back(lit1);
                lits.push_back(lit2);
            }

            const bool getRhs() const
            {
                return rhs;
            }

            const size_t size() const
            {
                return lits.size();
            }

            vector<Lit>::const_iterator begin() const
            {
                return lits.begin();
            }

            vector<Lit>::const_iterator end() const
            {
                return lits.end();
            }

            const vector<Lit>& getLits() const
            {
                return lits;
            }

        private:
            vector<Lit> lits;
            const bool rhs;
    };
    public:
        SolutionExtender(ThreadControl* _control, const vector<lbool>& _assigns);
        void extend();
        const bool addClause(const vector<Lit>& lits);
        void addBlockedClause(const BlockedClause& cl);
        void enqueue(const Lit lit);

        const lbool value(const Lit lit) const
        {
            return assigns[lit.var()] ^ lit.sign();
        }

        const lbool value(const Var var) const
        {
            return assigns[var];
        }

    private:
        const bool propagateCl(MyClause& cl);
        const bool propagate();
        const bool satisfiedNorm(const vector<Lit>& lits) const;
        const bool satisfiedXor(const vector<Lit>& lits, const bool rhs) const;
        const Lit pickBranchLit();

        const uint32_t nVars()
        {
            return assigns.size();
        }


        ThreadControl* control;
        vector<vector<MyClause*> > occur;
        vector<MyClause*> clauses;
        uint32_t qhead;
        vector<Lit> trail;
        vector<lbool> assigns;
};

#endif //__SOLUTIONEXTENDER_H__