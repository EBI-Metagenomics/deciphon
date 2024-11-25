from enum import Enum

from deciphon_core.schema import Gencode

from deciphon_gui.shorten import shorten


class GencodeName(Enum):
    SGC0 = "The Standard Code"
    SGC1 = "The Vertebrate Mitochondrial Code"
    SGC2 = "The Yeast Mitochondrial Code"
    SGC3 = "The Mold, Protozoan, and Coelenterate Mitochondrial Code and the Mycoplasma/Spiroplasma Code"
    SGC4 = "The Invertebrate Mitochondrial Code"
    SGC5 = "The Ciliate, Dasycladacean and Hexamita Nuclear Code"
    SGC8 = "The Echinoderm and Flatworm Mitochondrial Code"
    SGC9 = "The Euplotid Nuclear Code"
    BAPP = "The Bacterial, Archaeal and Plant Plastid Code"
    AYN = "The Alternative Yeast Nuclear Code"
    AMC = "The Ascidian Mitochondrial Code"
    AFMC = "The Alternative Flatworm Mitochondrial Code"
    BMN = "Blepharisma Nuclear Code"
    CMC = "Chlorophycean Mitochondrial Code"
    TMC = "Trematode Mitochondrial Code"
    SOMC = "Scenedesmus obliquus Mitochondrial Code"
    TMMC = "Thraustochytrium Mitochondrial Code"
    PMMC = "Rhabdopleuridae Mitochondrial Code"
    CDSR1G = "Candidate Division SR1 and Gracilibacteria Code"
    PTN = "Pachysolen tannophilus Nuclear Code"
    KN = "Karyorelict Nuclear Code"
    CN = "Condylostoma Nuclear Code"
    MN = "Mesodinium Nuclear Code"
    PN = "Peritrich Nuclear Code"
    BN = "Blastocrithidia Nuclear Code"
    BP = "Balanophoraceae Plastid Code"
    CMMC = "Cephalodiscidae Mitochondrial UAA-Tyr Code"


def gencode_description(code: Gencode, name: GencodeName):
    return f"{code.value}. {shorten(name.value, 46)}"
