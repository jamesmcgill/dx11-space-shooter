xof 0303txt 0032
template XSkinMeshHeader {
 <3cf169ce-ff7c-44ab-93c0-f78f62d172e2>
 WORD nMaxSkinWeightsPerVertex;
 WORD nMaxSkinWeightsPerFace;
 WORD nBones;
}

template VertexDuplicationIndices {
 <b8d65549-d7c9-4995-89cf-53a9a8b031e3>
 DWORD nIndices;
 DWORD nOriginalVertices;
 array DWORD indices[nIndices];
}

template SkinWeights {
 <6f0d123b-bad2-4167-a0d0-80224f25fabb>
 STRING transformNodeName;
 DWORD nWeights;
 array DWORD vertexIndices[nWeights];
 array FLOAT weights[nWeights];
 Matrix4x4 matrixOffset;
}


Frame Scene_Root {
 

 Frame Camera_Root {
  

  FrameTransformMatrix {
   1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,2.000000,20.000000,1.000000;;
  }

  Frame Camera {
   

   FrameTransformMatrix {
    1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000;;
   }
  }
 }

 Frame light {
  

  FrameTransformMatrix {
   1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000;;
  }
 }

 Frame cube {
  

  FrameTransformMatrix {
   1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000,0.000000,0.000000,0.000000,0.000000,1.000000;;
  }

  Mesh cube_obj {
   54;
   0.163822;0.142664;-0.070795;,
   0.118503;0.108081;-0.070795;,
   0.090515;0.039117;-0.044569;,
   0.054607;0.096754;-0.044569;,
   0.006893;-0.138145;-0.044569;,
   -0.006893;-0.138145;-0.044569;,
   -0.054607;0.096754;-0.044569;,
   -0.090515;0.039117;-0.044569;,
   -0.118503;0.108081;-0.070795;,
   -0.163822;0.142664;-0.070795;,
   0.163822;0.142664;-0.069308;,
   0.118503;0.108081;-0.067714;,
   0.090515;0.039117;-0.039546;,
   0.054607;0.096754;-0.026770;,
   0.006893;-0.138145;-0.042318;,
   -0.006893;-0.138145;-0.042318;,
   -0.054607;0.096754;-0.026770;,
   -0.090515;0.039117;-0.039546;,
   -0.118503;0.108081;-0.067714;,
   -0.163822;0.142664;-0.069308;,
   0.163822;0.161810;-0.070795;,
   0.118503;0.161810;-0.070795;,
   0.091012;0.161810;-0.044569;,
   0.054607;0.161810;-0.044569;,
   0.018202;0.135088;-0.044569;,
   -0.018202;0.135088;-0.044569;,
   -0.054607;0.161810;-0.044569;,
   -0.091012;0.161810;-0.044569;,
   -0.118503;0.161810;-0.070795;,
   -0.163822;0.161810;-0.070795;,
   0.163822;0.161810;-0.069308;,
   0.118503;0.161810;-0.067714;,
   0.091012;0.161810;-0.039546;,
   0.054607;0.125596;-0.026770;,
   0.018202;0.088212;-0.011377;,
   -0.018202;0.088212;-0.011377;,
   -0.054607;0.125596;-0.026770;,
   -0.091012;0.161810;-0.039546;,
   -0.118503;0.161810;-0.067714;,
   -0.163822;0.161810;-0.069308;,
   -0.014786;0.019830;-0.020724;,
   0.014782;0.019759;-0.020734;,
   -0.008777;-0.100881;-0.037163;,
   0.008755;-0.100872;-0.037223;,
   0.054607;0.114439;-0.026770;,
   -0.054607;0.115546;-0.026770;,
   -0.090825;0.115545;-0.039546;,
   0.090821;0.114546;-0.039546;,
   -0.118503;0.123217;-0.067714;,
   0.118503;0.121549;-0.067714;,
   0.090754;0.098204;-0.039546;,
   -0.090754;0.098207;-0.039546;,
   -0.017053;0.065213;-0.014521;,
   0.016687;0.065214;-0.014612;;
   55;
   4;41,40,42,43;,
   4;22,23,33,32;,
   4;26,27,37,36;,
   3;10,49,11;,
   3;18,48,19;,
   4;46,48,18,51;,
   4;45,46,51,16;,
   4;52,45,16,40;,
   4;44,53,41,13;,
   4;47,44,13,50;,
   4;49,47,50,11;,
   4;40,41,53,52;,
   4;0,10,11,1;,
   4;0,1,21,20;,
   4;0,20,30,10;,
   4;1,11,12,2;,
   4;1,2,22,21;,
   4;2,12,13,3;,
   4;2,3,23,22;,
   4;3,13,14,4;,
   4;3,4,24,23;,
   4;4,14,15,5;,
   4;4,5,25,24;,
   4;5,15,16,6;,
   4;5,6,26,25;,
   4;6,16,17,7;,
   4;6,7,27,26;,
   4;7,17,18,8;,
   4;7,8,28,27;,
   4;8,18,19,9;,
   4;8,9,29,28;,
   4;9,19,39,29;,
   3;11,50,12;,
   3;12,50,13;,
   4;13,41,43,14;,
   4;14,43,42,15;,
   4;15,42,40,16;,
   3;16,51,17;,
   3;17,51,18;,
   4;20,21,31,30;,
   4;21,22,32,31;,
   4;23,24,34,33;,
   4;24,25,35,34;,
   4;25,26,36,35;,
   4;27,28,38,37;,
   4;28,29,39,38;,
   4;34,35,52,53;,
   4;30,31,49,10;,
   4;31,32,47,49;,
   4;32,33,44,47;,
   4;33,34,53,44;,
   4;35,36,45,52;,
   4;36,37,46,45;,
   4;37,38,48,46;,
   4;38,39,19,48;;

   MeshNormals {
    214;
    0.896282;-0.443485;0.000000;,
    0.896282;-0.443485;0.000000;,
    0.745878;-0.292812;0.598270;,
    0.794721;-0.606974;0.000000;,
    0.000000;0.000000;-1.000000;,
    -0.370503;0.001044;-0.928831;,
    -0.370503;0.001044;-0.928831;,
    0.000000;0.000000;-1.000000;,
    0.896282;-0.443485;0.000000;,
    1.000000;0.000000;0.000000;,
    1.000000;0.000000;0.000000;,
    0.896282;-0.443485;0.000000;,
    0.794721;-0.606974;0.000000;,
    0.745878;-0.292812;0.598270;,
    0.759911;-0.146487;0.633306;,
    0.926599;-0.376050;0.000000;,
    -0.370503;0.001044;-0.928831;,
    -0.370503;0.001044;-0.928831;,
    -0.370503;0.001044;-0.928831;,
    -0.370503;0.001044;-0.928831;,
    -0.848762;-0.528776;0.000000;,
    -0.848762;-0.528776;0.000000;,
    -0.848762;-0.528776;0.000000;,
    -0.848762;-0.528776;0.000000;,
    -0.370503;0.001044;-0.928831;,
    0.000000;0.000000;-1.000000;,
    0.000000;0.000000;-1.000000;,
    -0.370503;0.001044;-0.928831;,
    0.979987;-0.199061;0.000000;,
    0.979987;-0.199061;0.000000;,
    0.979987;-0.199061;0.000000;,
    0.979987;-0.199061;0.000000;,
    0.000000;0.000000;-1.000000;,
    0.000000;0.000000;-1.000000;,
    0.000000;0.000000;-1.000000;,
    0.000000;0.000000;-1.000000;,
    0.000000;-1.000000;0.000000;,
    0.000000;-1.000000;0.000000;,
    0.000000;-1.000000;0.000000;,
    0.000000;-1.000000;0.000000;,
    0.000000;0.000000;-1.000000;,
    0.000000;0.000000;-1.000000;,
    0.000000;0.000000;-1.000000;,
    0.000000;0.000000;-1.000000;,
    -0.979987;-0.199061;0.000000;,
    -0.979987;-0.199061;0.000000;,
    -0.979987;-0.199061;0.000000;,
    -0.979987;-0.199061;0.000000;,
    0.000000;0.000000;-1.000000;,
    0.000000;0.000000;-1.000000;,
    0.000000;0.000000;-1.000000;,
    0.000000;0.000000;-1.000000;,
    0.848762;-0.528776;0.000000;,
    0.848762;-0.528776;0.000000;,
    0.848762;-0.528776;0.000000;,
    0.848762;-0.528776;0.000000;,
    0.000000;0.000000;-1.000000;,
    0.370503;0.001044;-0.928831;,
    0.370503;0.001044;-0.928831;,
    0.000000;0.000000;-1.000000;,
    -0.926599;-0.376050;0.000000;,
    -0.759911;-0.146487;0.633306;,
    -0.745887;-0.292802;0.598264;,
    -0.794721;-0.606974;0.000000;,
    0.370503;0.001044;-0.928831;,
    0.370503;0.001044;-0.928831;,
    0.370503;0.001044;-0.928831;,
    0.370503;0.001044;-0.928831;,
    -0.794721;-0.606974;0.000000;,
    -0.745887;-0.292802;0.598264;,
    -0.896282;-0.443485;0.000000;,
    -0.896282;-0.443485;0.000000;,
    0.370503;0.001044;-0.928831;,
    0.000000;0.000000;-1.000000;,
    0.000000;0.000000;-1.000000;,
    0.370503;0.001044;-0.928831;,
    -0.896282;-0.443485;0.000000;,
    -0.896282;-0.443485;0.000000;,
    -1.000000;0.000000;0.000000;,
    -1.000000;0.000000;0.000000;,
    0.035150;0.000000;0.999382;,
    0.035150;0.000000;0.999382;,
    0.403704;-0.000842;0.914889;,
    0.745878;-0.292812;0.598270;,
    0.745878;-0.292812;0.598270;,
    0.403704;-0.000842;0.914889;,
    0.304238;0.498988;0.811449;,
    0.759911;-0.146487;0.633306;,
    0.759911;-0.146487;0.633306;,
    0.304238;0.498988;0.811449;,
    0.109098;0.255397;0.960661;,
    0.365669;-0.061713;0.928697;,
    0.365669;-0.061713;0.928697;,
    0.109098;0.255397;0.960661;,
    0.050119;0.232226;0.971370;,
    0.212661;-0.143117;0.966588;,
    0.212661;-0.143117;0.966588;,
    0.050119;0.232226;0.971370;,
    -0.049486;0.233239;0.971159;,
    -0.211559;-0.143193;0.966819;,
    -0.211559;-0.143193;0.966819;,
    -0.049486;0.233239;0.971159;,
    -0.109150;0.256378;0.960394;,
    -0.367660;-0.062122;0.927883;,
    -0.367660;-0.062122;0.927883;,
    -0.109150;0.256378;0.960394;,
    -0.304250;0.498983;0.811448;,
    -0.759911;-0.146487;0.633306;,
    -0.759911;-0.146487;0.633306;,
    -0.304250;0.498983;0.811448;,
    -0.403727;-0.000850;0.914879;,
    -0.745887;-0.292802;0.598264;,
    -0.745887;-0.292802;0.598264;,
    -0.403727;-0.000850;0.914879;,
    -0.035150;0.000000;0.999382;,
    -0.035150;0.000000;0.999382;,
    0.000000;1.000000;0.000000;,
    0.000000;1.000000;0.000000;,
    0.000000;1.000000;0.000000;,
    0.000000;1.000000;0.000000;,
    0.000000;1.000000;0.000000;,
    -0.066295;0.874518;0.480441;,
    0.304238;0.498988;0.811449;,
    0.000000;1.000000;0.000000;,
    -0.066295;0.874518;0.480441;,
    -0.193536;0.518202;0.833073;,
    0.109098;0.255397;0.960661;,
    0.304238;0.498988;0.811449;,
    -0.193536;0.518202;0.833073;,
    -0.136301;0.546131;0.826537;,
    0.050119;0.232226;0.971370;,
    0.109098;0.255397;0.960661;,
    -0.136301;0.546131;0.826537;,
    0.136301;0.546131;0.826537;,
    -0.049486;0.233239;0.971159;,
    0.050119;0.232226;0.971370;,
    0.136301;0.546131;0.826537;,
    0.193536;0.518202;0.833073;,
    -0.109150;0.256378;0.960394;,
    -0.049486;0.233239;0.971159;,
    0.193536;0.518202;0.833073;,
    0.066295;0.874518;0.480441;,
    -0.304250;0.498983;0.811448;,
    -0.109150;0.256378;0.960394;,
    0.066295;0.874518;0.480441;,
    0.000000;1.000000;0.000000;,
    0.000000;1.000000;0.000000;,
    -0.304250;0.498983;0.811448;,
    0.000000;1.000000;0.000000;,
    0.000000;1.000000;0.000000;,
    0.000000;1.000000;0.000000;,
    0.000000;1.000000;0.000000;,
    -0.203781;-0.131685;0.970120;,
    -0.203781;-0.131685;0.970120;,
    0.203022;-0.131181;0.970347;,
    0.203022;-0.131181;0.970347;,
    -0.140223;-0.141210;0.979999;,
    -0.140223;-0.141210;0.979999;,
    0.141852;-0.141151;0.979773;,
    0.141852;-0.141151;0.979773;,
    0.203022;-0.131181;0.970347;,
    -0.203781;-0.131685;0.970120;,
    0.141852;-0.141151;0.979773;,
    -0.140223;-0.141210;0.979999;,
    0.374512;-0.051725;0.925778;,
    0.374512;-0.051725;0.925778;,
    -0.376186;-0.051157;0.925131;,
    -0.376186;-0.051157;0.925131;,
    -0.537134;-0.001251;0.843496;,
    -0.537134;-0.001251;0.843496;,
    0.537120;-0.001254;0.843505;,
    0.537120;-0.001254;0.843505;,
    -0.403237;-0.000840;0.915095;,
    -0.403237;-0.000840;0.915095;,
    0.403216;-0.000847;0.915104;,
    0.403216;-0.000847;0.915104;,
    0.536824;-0.001660;0.843693;,
    0.536824;-0.001660;0.843693;,
    -0.536828;-0.001649;0.843690;,
    -0.536828;-0.001649;0.843690;,
    -0.213212;-0.121411;0.969433;,
    -0.213212;-0.121411;0.969433;,
    0.212747;-0.121918;0.969471;,
    0.212747;-0.121918;0.969471;,
    0.035150;0.000000;0.999382;,
    0.403216;-0.000847;0.915104;,
    0.403216;-0.000847;0.915104;,
    0.537120;-0.001254;0.843505;,
    0.537120;-0.001254;0.843505;,
    0.374512;-0.051725;0.925778;,
    0.374512;-0.051725;0.925778;,
    0.212747;-0.121918;0.969471;,
    -0.213212;-0.121411;0.969433;,
    -0.376186;-0.051157;0.925131;,
    -0.376186;-0.051157;0.925131;,
    -0.537134;-0.001251;0.843496;,
    -0.537134;-0.001251;0.843496;,
    -0.403237;-0.000840;0.915095;,
    -0.403237;-0.000840;0.915095;,
    -0.035150;0.000000;0.999382;,
    -0.536828;-0.001649;0.843690;,
    -0.745887;-0.292802;0.598264;,
    -0.367660;-0.062122;0.927883;,
    -0.536828;-0.001649;0.843690;,
    -0.203781;-0.131685;0.970120;,
    -0.367660;-0.062122;0.927883;,
    0.365669;-0.061713;0.928697;,
    0.203022;-0.131181;0.970347;,
    0.536824;-0.001660;0.843693;,
    0.365669;-0.061713;0.928697;,
    0.745878;-0.292812;0.598270;,
    0.536824;-0.001660;0.843693;,
    -0.213212;-0.121411;0.969433;,
    0.212747;-0.121918;0.969471;;
    55;
    4;155,152,163,162;,
    4;124,125,126,127;,
    4;140,141,142,143;,
    3;80,174,83;,
    3;112,173,115;,
    4;169,172,201,200;,
    4;167,168,203,202;,
    4;181,166,205,204;,
    4;165,182,207,206;,
    4;171,164,209,208;,
    4;175,170,211,210;,
    4;161,160,213,212;,
    4;0,1,2,3;,
    4;4,5,6,7;,
    4;8,9,10,11;,
    4;12,13,14,15;,
    4;16,17,18,19;,
    4;20,21,22,23;,
    4;24,25,26,27;,
    4;28,29,30,31;,
    4;32,33,34,35;,
    4;36,37,38,39;,
    4;40,41,42,43;,
    4;44,45,46,47;,
    4;48,49,50,51;,
    4;52,53,54,55;,
    4;56,57,58,59;,
    4;60,61,62,63;,
    4;64,65,66,67;,
    4;68,69,70,71;,
    4;72,73,74,75;,
    4;76,77,78,79;,
    3;84,176,87;,
    3;88,177,91;,
    4;92,154,158,95;,
    4;96,159,156,99;,
    4;100,157,153,103;,
    3;104,178,107;,
    3;108,179,111;,
    4;116,117,118,119;,
    4;120,121,122,123;,
    4;128,129,130,131;,
    4;132,133,134,135;,
    4;136,137,138,139;,
    4;144,145,146,147;,
    4;148,149,150,151;,
    4;97,98,180,183;,
    4;81,82,185,184;,
    4;85,86,187,186;,
    4;89,90,189,188;,
    4;93,94,191,190;,
    4;101,102,193,192;,
    4;105,106,195,194;,
    4;109,110,197,196;,
    4;113,114,199,198;;
   }

   MeshMaterialList {
    4;
    55;
    0,
    1,
    1,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    2,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3;

    Material cubePolygon2 {
     0.000000;0.032000;0.190000;1.000000;;
     50.000000;
     1.000000;1.000000;1.000000;;
     0.000000;0.000000;0.000000;;
    }

    Material cubePolygon3 {
     1.000000;0.890000;0.000000;1.000000;;
     50.000000;
     1.000000;1.000000;1.000000;;
     0.000000;0.000000;0.000000;;
    }

    Material cubePolygon {
     1.000000;0.000000;0.000000;1.000000;;
     50.000000;
     1.000000;1.000000;1.000000;;
     0.000000;0.000000;0.000000;;
    }

    Material DefaultLib_Material11 {
     0.890000;0.890000;0.890000;1.000000;;
     50.000000;
     1.000000;1.000000;1.000000;;
     0.000000;0.000000;0.000000;;
    }
   }
  }
 }
}