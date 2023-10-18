#ifndef TEST_CONSENSUS_H
#define TEST_CONSENSUS_H

static struct
{
  long id;
  char const *name;
  char const *data;
} sequences[] = {
    {1, "Homoserine_dh-consensus",
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "ACAGGAATTGGGCTACGCGGAAGCGGATCCTACGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"},
    {2, "AA_kinase-consensus",
     "AAACGTGTAGTTGTAAAGCTTGGGGGTAGTTCTCTGACAGATAAGGAAGAGGCATCACTCAGGCGTTTAG"
     "CTGAGCAGATTGCAGCATTAAAAGAGAGTGGCAATAAACTAGTGGTCGTGCATGGAGGCGGCAGCTTCAC"
     "TGATGGTCTGCTGGCATTGAAAAGTGGCCTGAGCTCGGGCGAATTAGCTGCGGGGTTGAGGAGCACGTTA"
     "GAAGAGGCCGGAGAAGTAGCGACGAGGGACGCCCTAGCTAGCTTAGGGGAACGGCTTGTTGCAGCGCTGC"
     "TGGCGGCGGGTCTCCCTGCTGTAGGACTCAGCGCCGCTGCGTTAGATGCGACGGAGGCGGGCCGGGATGA"
     "AGGCAGCGACGGGAACGTCGAGTCCGTGGACGCAGAAGCAATTGAGGAGTTGCTTGAGGCCGGGGTGGTC"
     "CCCGTCCTAACAGGATTTATCGGCTTAGACGAAGAAGGGGAACTGGGAAGGGGATCTTCTGACACCATCG"
     "CTGCGTTACTCGCTGAAGCTTTAGGCGCGGACAAACTCATAATACTGACCGACGTAGACGGCGTTTACGA"
     "TGCCGACCCTAAAAAGGTCCCAGACGCGAGGCTCTTGCCAGAGATAAGTGTGGACGAGGCCGAGGAAAGC"
     "GCCTCCGAATTAGCGACCGGTGGGATGAAGGTCAAACATCCAGCGGCTCTTGCTGCAGCTAGACGGGGGG"
     "GTATTCCGGTCGTGATAACGAAT"},
    {3, "23ISL-consensus",
     "CAGGGTCTGGATAACGCTAATCGTTCGCTAGTTCGCGCTACAAAAGCAGAAAGTTCAGATATACGGAAAG"
     "AGGTGACTAACGGCATCGCTAAAGGGCTGAAGCTAGACAGTCTGGAAACAGCTGCAGAGTCGAAGAACTG"
     "CTCAAGCGCACAGAAAGGCGGATCGCTAGCTTGGGCAACCAACTCCCAACCACAGCCTCTCCGTGAAAGT"
     "AAGCTTGAGCCATTGGAAGACTCCCCACGTAAGGCTTTAAAAACACCTGTGTTGCAAAAGACATCCAGTA"
     "CCATAACTTTACAAGCAGTCAAGGTTCAACCTGAACCCCGCGCTCCCGTCTCCGGGGCGCTGTCCCCGAG"
     "CGGGGAGGAACGCAAGCGCCCAGCTGCGTCTGCTCCCGCTACCTTACCGACACGACAGAGTGGTCTAGGT"
     "TCTCAGGAAGTCGTTTCGAAGGTGGCGACTCGCAAAATTCCAATGGAGTCACAACGCGAGTCGACT"},
    {4, "Homoserine_dh-multi-consensus1",
     "GG"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "ACAGGAATTGGGCTACGCGGAAGCGGATCCTACGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"
     "CATAGGG"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "ACAGGAATTGGGCTACGCGGAAGCGGATCCTACGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"
     "TT"},
    {5, "Homoserine_dh-multi-consensus2",
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "ACAGGAATTGGGCTACGCGGAAGCGGATCCTACGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "ACAGGAATTGGGCTACGCGGAAGCGGATCCTACGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"},
    {6, "Homoserine_dh-multi-consensus3",
     "GG"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "ACAGGAATTGGGCTACGCGGAAGCGGATCCTTCGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGCGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"
     "CATTTAGGG"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "ACAGGAATTGGGCTACGCGGAAGCGGATCCTACGGACGAAAAAAAAAAAAAAAAAGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"
     "TT"},
    {7, "Homoserine_dh-multi-consensus3",
     "GG"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "ACAGGAATTGGGCTACGCGGAAGCGGATCCTTCGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGCGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"
     "CATTTAGGG"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "CTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "AATTGGGCTACGCGGAAGCGGATCCTACGGACGAAAAAAAGATGGAGAAAAAAAAAAGCTTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"
     "TT"},
    {8, "Homoserine_dh-multi-consensus1",
     "GG"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGGCTTCATTCTCTGAGGCGCTGAAGGAGGC"
     "ACAGAATTGGGCTACGCGGAAGCGGATCCTACGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGTGGCAAGCGCCGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGCTGTAGAAACGGAACGGGTAGGCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTGACCTTCTC"
     "CATAGG"
     "CCTATCATTTCGACGCTCAAGGAGTCGCTGACAGGTGACCGTATTACTCGAATCGAAGGGATATTAAACG"
     "GCACCCTGAATTACATTCTCACTGAGATGGAGGAAGAGGGGAAAAAAAAAAAAAAGGCGCTGAAGGAGGC"
     "ACAGGATTGGGCTACGCGGAAGCGGATCCTACGGACGATGTGGAAGGGCTAGATGCTGCTAGAAAGCTG"
     "GCAATTCTAGCCAGATTGGCATTTGGGTTAGAGGTCGAGTTGGAGGACGTAGAGGTGGAAGGAATTGAAA"
     "AGCTGACTGCCGAAGATATTGGCTTCGGCTTCGAAGAAGCGAAGGAAGAGGGTAAAGTTTTAAAACTAGT"
     "CGAAGCCAGGGTCAAGCCTGAGCTGGTACCTAAGTCACATCATTAGCCTCGGTAAAAGGCTCTGACAAC"
     "GCCGTGGCTGTAGAAACGGAACGGCCCCCCGAACTCGTAGTGCAGGGACCAGGGGCTGGCGCAGAGCCAA"
     "CCGCATCCGCTGTACTCGCTTTTTTTCTC"
     "TT"}};

#endif
