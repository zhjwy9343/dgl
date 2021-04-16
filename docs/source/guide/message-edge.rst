.. _guide-message-passing-edge:

2.4 Apply Edge Weight In Message Passing
----------------------------------------

:ref:`(中文版) <guide_cn-message-passing-edge>`

A commonly seen practice in GNN modeling is to apply edge weight on the
message before message aggregation, for examples, in
`GAT <https://arxiv.org/pdf/1710.10903.pdf>`__ and some `GCN
variants <https://arxiv.org/abs/2004.00445>`__. In DGL, the way to
handle this is:

-  Save the weight as edge feature.
-  Multiply the edge feature by src node feature in message function.

For example:

.. code::

    import dgl.function as fn

    # Suppose eweight is a tensor of shape (E, *), where E is the number of edges.
    graph.edata['a'] = eweight
    graph.update_all(fn.u_mul_e('ft', 'a', 'm'),
                     fn.sum('m', 'ft'))

The example above uses eweight as the edge weight. The edge weight should
usually be a scalar.