/*
============================================================================
DELLY: Structural variant discovery by integrated PE mapping and SR analysis
============================================================================
Copyright (C) 2012 Tobias Rausch

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
============================================================================
Contact: Tobias Rausch (rausch@embl.de)
============================================================================
*/

#ifndef TAGS_H
#define TAGS_H

namespace torali {

  // Constants
  #define MAX_CHROM_SIZE 250000000


  // Tags
  struct DeletionTag;
  struct DuplicationTag;
  struct InversionTag;

  template<typename SvTag>
    struct SVType {
    };


  // F+ 0
  // F- 1
  // R+ 2
  // R- 3

  template<typename TBamRecord>
  inline int
    getStrandIndependentOrientation(TBamRecord const& al) {
    if (al.AlignmentFlag & 0x0040) {
      if (!(al.AlignmentFlag & 0x0010)) {
	if (!(al.AlignmentFlag & 0x0020)) return (al.Position < al.MatePosition) ? 0 : 1;
	else return (al.Position < al.MatePosition) ? 2 : 3;
      } else {
	if (!(al.AlignmentFlag & 0x0020)) return (al.Position > al.MatePosition) ? 2 : 3;
	else return (al.Position > al.MatePosition) ? 0 : 1;
      }
    } else {
      if (!(al.AlignmentFlag & 0x0010)) {
	if (!(al.AlignmentFlag & 0x0020)) return (al.Position < al.MatePosition) ? 1 : 0;
	else return (al.Position < al.MatePosition) ? 2 : 3;
      } else {
	if (!(al.AlignmentFlag & 0x0020)) return (al.Position > al.MatePosition) ? 2 : 3;
	else return (al.Position > al.MatePosition) ? 1 : 0;
      }
    }
  }

  // Left- or right-spanning
  template<typename TBamRecord>
    inline bool 
    _getSpanOrientation(TBamRecord const& al, int const defaultOrient, SVType<InversionTag>) {
    int orient = getStrandIndependentOrientation(al);
    if (al.AlignmentFlag & 0x0040) {
      if (defaultOrient == 0) {
	if (((orient==2) && (al.Position < al.MatePosition)) || ((orient == 3) && (al.Position > al.MatePosition))) return true;
      } else if (defaultOrient == 1) {
	if (((orient==2) && (al.Position > al.MatePosition)) || ((orient == 3) && (al.Position < al.MatePosition))) return true;
      } else if (defaultOrient == 2) {
	if (((orient==0) && (al.Position < al.MatePosition)) || ((orient == 1) && (al.Position > al.MatePosition))) return true;
      } else if (defaultOrient == 3) {
	if (((orient==0) && (al.Position > al.MatePosition)) || ((orient == 1) && (al.Position < al.MatePosition))) return true;
      }
      return false;
    } else {
      if (defaultOrient == 0) {
	if (((orient==2) && (al.Position > al.MatePosition)) || ((orient == 3) && (al.Position < al.MatePosition))) return true;
      } else if (defaultOrient == 1) {
	if (((orient==2) && (al.Position < al.MatePosition)) || ((orient == 3) && (al.Position > al.MatePosition))) return true;
      } else if (defaultOrient == 2) {
	if (((orient==0) && (al.Position > al.MatePosition)) || ((orient == 1) && (al.Position < al.MatePosition))) return true;
      } else if (defaultOrient == 3) {
	if (((orient==0) && (al.Position < al.MatePosition)) || ((orient == 1) && (al.Position > al.MatePosition))) return true;
      }
      return false;
    }
  }

  // Dummy function for all other SV types
  template<typename TBamRecord, typename TTag>
    inline bool 
    _getSpanOrientation(TBamRecord const&, int const, SVType<TTag>) {
    return true;
  }

  // Unique paired-end data structure for single chromosome only
  struct Hit {
    int32_t minPos;
    int32_t maxPos;
  
    template<typename TBamRecord>
  Hit(TBamRecord const& al) : minPos(std::min(al.Position, al.MatePosition)), maxPos(std::max(al.Position, al.MatePosition)) {}

    bool operator <(Hit const& other) const {
      return ((minPos<other.minPos) || ((minPos==other.minPos) && (maxPos<other.maxPos)));
    }
  };

  // Structural variant record
  struct StructuralVariantRecord {
    int svStartBeg;
    int svStartEnd;
    int svEndBeg;
    int svEndEnd;
    int svStart;
    int svEnd;
    int peSupport;
    int srSupport;
    int wiggle;
    double srAlignQuality;
    unsigned int id;
    bool precise;
    bool left;
    uint16_t peMapQuality;
    std::string chr;
    std::string consensus;

    StructuralVariantRecord() {}
  StructuralVariantRecord(std::string const& c, int s, int e) : svStart(s), svEnd(e), chr(c) {}
  };

  template<typename TSV>
  struct SortSVs : public std::binary_function<TSV, TSV, bool>
  {
    inline bool operator()(TSV const& sv1, TSV const& sv2) {
      return ((sv1.chr<sv2.chr) || ((sv1.chr==sv2.chr) && (sv1.svStart<sv2.svStart)) || ((sv1.chr==sv2.chr) && (sv1.svStart==sv2.svStart) && (sv1.svEnd<sv2.svEnd)));
    }
  };



  // SV Paired-end checks

  // Deletions
  template<typename TISize>
    inline bool
    _acceptedInsertSize(TISize const maxNormalISize, TISize const, TISize const iSize, SVType<DeletionTag>) {
    return (maxNormalISize > iSize);
  }

  // Duplications
  template<typename TISize>
    inline bool
    _acceptedInsertSize(TISize const, TISize const median, TISize const iSize, SVType<DuplicationTag>) {
    // Exclude the chimeras in mate-pair libraries
    return !((median<1000) || ((median>=1000) && (iSize >=1000)));
  }

  // Other SV Types
  template<typename TISize, typename TTag>
    inline bool
    _acceptedInsertSize(TISize const, TISize const, TISize const, SVType<TTag>) {
    return false;
  }

  // Deletions
  template<typename TOrientation>
  inline bool
    _acceptedOrientation(TOrientation const def, TOrientation const lib, SVType<DeletionTag>) {
    return (def != lib);
  }

  // Duplications
  template<typename TOrientation>
    inline bool
    _acceptedOrientation(TOrientation const def, TOrientation const lib, SVType<DuplicationTag>) {
    if (def==0) return (lib != 1);
    else if (def==1) return (lib!=0);
    else if (def==2) return (lib!=3);
    else if (def==3) return (lib!=2);
    else return true;
  }

  // Inversion
  template<typename TOrientation>
    inline bool
    _acceptedOrientation(TOrientation const def, TOrientation const lib, SVType<InversionTag>) {
    return (((def<=1) && (lib!=2) && (lib!=3)) || ((def>=2) && (lib!=0) && (lib!=1)));
  }

  // Other SV Types
  template<typename TOrientation, typename TTag>
    inline bool
    _acceptedOrientation(TOrientation const def, TOrientation const lib, SVType<TTag>) {
    return false;
  }


  // Deletions
  template<typename TSize, typename TISize>
    inline bool
    _pairsDisagree(TSize const pair1Min, TSize const pair1Max, TSize const pair1ReadLength, TISize const pair1maxNormalISize, TSize const pair2Min, TSize const pair2Max, TSize const pair2ReadLength, TISize const pair2maxNormalISize, bool const, bool const, SVType<DeletionTag>) {
    //std::cout << pair1Min << ',' << pair1Max << ',' << pair1ReadLength << ',' << pair1maxNormalISize << ',' << pair2Min << ',' << pair2Max << ',' << pair2ReadLength << ',' << pair2maxNormalISize << std::endl;
    if ((pair2Min + pair2ReadLength - pair1Min) > pair1maxNormalISize) return true;
    if ((pair2Max < pair1Max) && ((pair1Max + pair1ReadLength - pair2Max) > pair1maxNormalISize)) return true;
    if ((pair2Max >= pair1Max) && ((pair2Max + pair2ReadLength - pair1Max) > pair2maxNormalISize)) return true;
    return false;
  }

  // Duplications
  template<typename TSize, typename TISize>
    inline bool
    _pairsDisagree(TSize const pair1Min, TSize const pair1Max, TSize const pair1ReadLength, TISize const pair1maxNormalISize, TSize const pair2Min, TSize const pair2Max, TSize const pair2ReadLength, TISize const pair2maxNormalISize, bool const, bool const, SVType<DuplicationTag>) {
    if ((pair2Min + pair2ReadLength - pair1Min) > pair2maxNormalISize) return true;
    if ((pair2Max < pair1Max) && ((pair1Max + pair1ReadLength - pair2Max) > pair2maxNormalISize)) return true;
    if ((pair2Max >= pair1Max) && ((pair2Max + pair2ReadLength - pair1Max) > pair1maxNormalISize)) return true;
    return false;
  }


  // Inversions
  template<typename TSize, typename TISize>
    inline bool
    _pairsDisagree(TSize const pair1Min, TSize const pair1Max, TSize const pair1ReadLength, TISize const pair1maxNormalISize, TSize const pair2Min, TSize const pair2Max, TSize const pair2ReadLength, TISize const pair2maxNormalISize, bool const pair1Left, bool const pair2Left, SVType<InversionTag>) {
    // Do both pairs support the same inversion type (left- or right-spanning)
    if (pair1Left != pair2Left) return true;
    if (pair1Left) {
      // Left-spanning inversions
      if ((pair2Min + pair2ReadLength - pair1Min) > pair1maxNormalISize) return true;
      if ((pair2Max < pair1Max) && ((pair1Max + pair1ReadLength - pair2Max) > pair2maxNormalISize)) return true;
      if ((pair2Max >= pair1Max) && ((pair2Max + pair2ReadLength - pair1Max) > pair1maxNormalISize)) return true;
    } else {
      // Right-spanning inversions
      if ((pair2Min + pair2ReadLength - pair1Min) > pair2maxNormalISize) return true;
      if ((pair2Max < pair1Max) && ((pair1Max + pair1ReadLength - pair2Max) > pair1maxNormalISize)) return true;
      if ((pair2Max >= pair1Max) && ((pair2Max + pair2ReadLength - pair1Max) > pair2maxNormalISize)) return true;
    }
    return false;
  }



}

#endif
